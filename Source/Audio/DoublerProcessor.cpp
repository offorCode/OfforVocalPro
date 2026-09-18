#include "DoublerProcessor.h"

#include <cmath>
#include <algorithm>


// ==========================================================
// CONSTRUCTOR
// ==========================================================

DoublerProcessor::DoublerProcessor()
{
}


// ==========================================================
// PREPARE
// ==========================================================

void DoublerProcessor::prepare(
    double newSampleRate,
    int newMaximumBlockSize,
    int newNumChannels)
{
    sampleRate =
        newSampleRate > 0.0
            ? newSampleRate
            : 44100.0;


    maximumBlockSize =
        juce::jmax(
            1,
            newMaximumBlockSize
        );


    numChannels =
        juce::jlimit(
            1,
            2,
            newNumChannels
        );


    // ======================================================
    // DELAY BUFFER
    // ======================================================

    delayBuffer.setSize(
        numChannels,
        delayBufferSize
    );

    delayBuffer.clear();


    prepared = true;


    // ======================================================
    // RESET DSP STATE
    // ======================================================

    reset();
}


// ==========================================================
// RESET
// ==========================================================

void DoublerProcessor::reset()
{
    clearDelayBuffer();

    writePosition = 0;


    // ======================================================
    // RESET LFO PHASE
    // ======================================================

    lfoPhaseLeft = 0.0;

    lfoPhaseRight = 0.0;


    // ======================================================
    // RESET LFO INTERPOLATION STATE
    // ======================================================

    leftLfoCurrent = 0.0f;
    leftLfoTarget = 0.0f;

    rightLfoCurrent = 0.0f;
    rightLfoTarget = 0.0f;

    leftLfoSamplesRemaining = 0;
    rightLfoSamplesRemaining = 0;


    leftLfoUpdateInterval =
        getEffectiveLfoUpdateInterval();

    rightLfoUpdateInterval =
        leftLfoUpdateInterval;


    // ======================================================
    // RESET PARAMETERS
    // ======================================================

    currentAmount = 0.0f;
    targetAmount = 0.0f;

    currentDetune = 0.0f;
    targetDetune = 0.0f;

    currentTiming = 0.0f;
    targetTiming = 0.0f;

    currentWidth = 0.0f;
    targetWidth = 0.0f;

    currentMix = 0.0f;
    targetMix = 0.0f;
}


// ==========================================================
// PROCESSING QUALITY
// ==========================================================

void DoublerProcessor::setProcessingQuality(
    ProcessingQuality newQuality)
{
    processingQuality = newQuality;


    // ======================================================
    // RECALCULATE LFO QUALITY
    //
    // We do not reset the audio buffer here.
    //
    // The new quality simply changes how frequently the LFO
    // calculation is performed.
    // ======================================================

    const int newInterval =
        getEffectiveLfoUpdateInterval();


    leftLfoUpdateInterval = newInterval;

    rightLfoUpdateInterval = newInterval;


    // Restart interpolation from the current LFO position.
    leftLfoSamplesRemaining = 0;
    rightLfoSamplesRemaining = 0;
}


// ==========================================================
// CPU MODE
// ==========================================================

void DoublerProcessor::setCPUMode(
    CPUMode newMode)
{
    cpuMode = newMode;


    // ======================================================
    // RECALCULATE DSP WORKLOAD
    // ======================================================

    const int newInterval =
        getEffectiveLfoUpdateInterval();


    leftLfoUpdateInterval = newInterval;

    rightLfoUpdateInterval = newInterval;


    leftLfoSamplesRemaining = 0;
    rightLfoSamplesRemaining = 0;
}


// ==========================================================
// BASE QUALITY
// ==========================================================
//
// This determines how often the expensive LFO sine
// calculation is performed.
//
// HIGH is deliberately the reference mode.
//
// ULTRA performs the LFO calculation every sample.
//
// LOW and MEDIUM calculate fewer points but interpolate
// between them to preserve smooth modulation.
//

int DoublerProcessor::getBaseLfoUpdateInterval() const
{
    switch (processingQuality)
    {
        case ProcessingQuality::Low:
            return 8;

        case ProcessingQuality::Medium:
            return 4;

        case ProcessingQuality::High:
            return 2;

        case ProcessingQuality::Ultra:
            return 1;

        default:
            return 2;
    }
}


// ==========================================================
// EFFECTIVE LFO UPDATE INTERVAL
// ==========================================================
//
// CPU MODE modifies the requested processing quality.
//
// LOW CPU:
//     More aggressive reduction in expensive calculations.
//
// BALANCED:
//     Uses the requested Processing Quality directly.
//
// PERFORMANCE:
//     Allows more frequent calculations.
//
// This does NOT bypass the user's quality selection.
// It only applies a CPU policy on top of it.
//

int DoublerProcessor::getEffectiveLfoUpdateInterval() const
{
    int interval =
        getBaseLfoUpdateInterval();


    switch (cpuMode)
    {
        case CPUMode::LowCPU:
        {
            // ----------------------------------------------
            // LOW CPU
            //
            // Double the calculation interval.
            //
            // Example:
            // High = 2 samples
            // becomes 4 samples.
            //
            // Ultra = 1 sample
            // becomes 2 samples.
            // ----------------------------------------------

            interval *= 2;

            break;
        }


        case CPUMode::Balanced:
        {
            // ----------------------------------------------
            // BALANCED
            //
            // Keep the requested quality.
            // ----------------------------------------------

            break;
        }


        case CPUMode::Performance:
        {
            // ----------------------------------------------
            // PERFORMANCE
            //
            // Allow a higher calculation rate.
            //
            // We never go below one calculation per sample.
            // ----------------------------------------------

            interval =
                juce::jmax(
                    1,
                    interval / 2
                );

            break;
        }
    }


    return juce::jlimit(
        1,
        16,
        interval
    );
}


// ==========================================================
// LEFT LFO
// ==========================================================
//
// Generates a smooth LFO value.
//
// At High/Ultra this is effectively sample-accurate.
//
// At lower quality levels, the expensive sine calculation
// happens less frequently, while the output is interpolated
// between calculated points.
//

float DoublerProcessor::getNextLeftLfoValue()
{
    const int interval =
        juce::jmax(
            1,
            leftLfoUpdateInterval
        );


    // ======================================================
    // START A NEW INTERPOLATION SEGMENT
    // ======================================================

    if (leftLfoSamplesRemaining <= 0)
    {
        leftLfoCurrent =
            sineLfo(
                lfoPhaseLeft
            );


        const double futurePhase =
            lfoPhaseLeft
            +
            (
                lfoRateLeft
                /
                sampleRate
            )
            *
            static_cast<double>(interval);


        leftLfoTarget =
            sineLfo(
                futurePhase
            );


        leftLfoSamplesRemaining =
            interval;
    }


    // ======================================================
    // CALCULATE INTERPOLATION POSITION
    // ======================================================

    const int samplesIntoSegment =
        interval
        -
        leftLfoSamplesRemaining;


    const float interpolation =
        static_cast<float>(
            samplesIntoSegment
        )
        /
        static_cast<float>(
            interval
        );


    const float output =
        leftLfoCurrent
        +
        (
            leftLfoTarget
            -
            leftLfoCurrent
        )
        *
        interpolation;


    // ======================================================
    // ADVANCE PHASE SAMPLE-BY-SAMPLE
    //
    // Even when the sine calculation is reduced, phase
    // remains sample accurate.
    // ======================================================

    lfoPhaseLeft +=
        lfoRateLeft
        /
        sampleRate;


    if (lfoPhaseLeft >= 1.0)
        lfoPhaseLeft -= 1.0;


    --leftLfoSamplesRemaining;


    return output;
}


// ==========================================================
// RIGHT LFO
// ==========================================================

float DoublerProcessor::getNextRightLfoValue()
{
    const int interval =
        juce::jmax(
            1,
            rightLfoUpdateInterval
        );


    // ======================================================
    // START A NEW INTERPOLATION SEGMENT
    // ======================================================

    if (rightLfoSamplesRemaining <= 0)
    {
        rightLfoCurrent =
            sineLfo(
                lfoPhaseRight
            );


        const double futurePhase =
            lfoPhaseRight
            +
            (
                lfoRateRight
                /
                sampleRate
            )
            *
            static_cast<double>(interval);


        rightLfoTarget =
            sineLfo(
                futurePhase
            );


        rightLfoSamplesRemaining =
            interval;
    }


    // ======================================================
    // CALCULATE INTERPOLATION POSITION
    // ======================================================

    const int samplesIntoSegment =
        interval
        -
        rightLfoSamplesRemaining;


    const float interpolation =
        static_cast<float>(
            samplesIntoSegment
        )
        /
        static_cast<float>(
            interval
        );


    const float output =
        rightLfoCurrent
        +
        (
            rightLfoTarget
            -
            rightLfoCurrent
        )
        *
        interpolation;


    // ======================================================
    // ADVANCE PHASE SAMPLE-BY-SAMPLE
    // ======================================================

    lfoPhaseRight +=
        lfoRateRight
        /
        sampleRate;


    if (lfoPhaseRight >= 1.0)
        lfoPhaseRight -= 1.0;


    --rightLfoSamplesRemaining;


    return output;
}


// ==========================================================
// CLEAR DELAY
// ==========================================================

void DoublerProcessor::clearDelayBuffer()
{
    if (delayBuffer.getNumSamples() > 0)
        delayBuffer.clear();
}


// ==========================================================
// PROCESS BLOCK
// ==========================================================

void DoublerProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    float amount,
    float detune,
    float timing,
    float width,
    float mix)
{
    if (!prepared)
        return;


    const int samples =
        buffer.getNumSamples();


    const int channels =
        juce::jmin(
            buffer.getNumChannels(),
            numChannels
        );


    if (samples <= 0 || channels <= 0)
        return;


    // ======================================================
    // NORMALIZE PARAMETERS
    // ======================================================

    targetAmount =
        juce::jlimit(
            0.0f,
            1.0f,
            amount / 100.0f
        );


    targetDetune =
        juce::jlimit(
            0.0f,
            1.0f,
            detune / 100.0f
        );


    targetTiming =
        juce::jlimit(
            0.0f,
            1.0f,
            timing / 100.0f
        );


    targetWidth =
        juce::jlimit(
            0.0f,
            1.0f,
            width / 100.0f
        );


    targetMix =
        juce::jlimit(
            0.0f,
            1.0f,
            mix / 100.0f
        );


    // ======================================================
    // PARAMETER SMOOTHING
    // ======================================================

    constexpr float smoothing =
        0.0025f;


    for (int sample = 0;
         sample < samples;
         ++sample)
    {
        currentAmount +=
            (
                targetAmount
                -
                currentAmount
            )
            *
            smoothing;


        currentDetune +=
            (
                targetDetune
                -
                currentDetune
            )
            *
            smoothing;


        currentTiming +=
            (
                targetTiming
                -
                currentTiming
            )
            *
            smoothing;


        currentWidth +=
            (
                targetWidth
                -
                currentWidth
            )
            *
            smoothing;


        currentMix +=
            (
                targetMix
                -
                currentMix
            )
            *
            smoothing;


        processSample(
            buffer,
            sample
        );
    }
}


// ==========================================================
// PROCESS SAMPLE
// ==========================================================

void DoublerProcessor::processSample(
    juce::AudioBuffer<float>& buffer,
    int sampleIndex)
{
    // ======================================================
    // READ INPUT
    // ======================================================

    float inputLeft = 0.0f;
    float inputRight = 0.0f;


    if (buffer.getNumChannels() >= 1)
    {
        inputLeft =
            buffer.getSample(
                0,
                sampleIndex
            );
    }


    if (buffer.getNumChannels() >= 2)
    {
        inputRight =
            buffer.getSample(
                1,
                sampleIndex
            );
    }
    else
    {
        inputRight = inputLeft;
    }


    // ======================================================
    // STORE INPUT IN DELAY BUFFER
    // ======================================================

    for (int channel = 0;
         channel < numChannels;
         ++channel)
    {
        const float input =
            channel == 0
                ? inputLeft
                : inputRight;


        delayBuffer.setSample(
            channel,
            writePosition,
            input
        );
    }


    // ======================================================
    // TIMING
    // ======================================================

    const float baseDelayMs =
        minimumDelayMs
        +
        (
            maximumDelayMs
            -
            minimumDelayMs
        )
        *
        currentTiming;


    // ======================================================
    // DETUNE
    //
    // Maximum modulation remains deliberately small.
    //
    // This preserves the natural "double take" character.
    // ======================================================

    const float modulationAmountMs =
        maximumModulationMs
        *
        currentDetune
        *
        currentAmount;


    // ======================================================
    // QUALITY-AWARE LFO
    //
    // These functions calculate/interpolate the LFO according
    // to Processing Quality + CPU Mode.
    //
    // The phase itself remains sample-accurate.
    // ======================================================

    const float leftLfo =
        getNextLeftLfoValue();


    const float rightLfo =
        getNextRightLfoValue();


    // ======================================================
    // MODULATED DELAYS
    // ======================================================

    const float leftDelayMs =
        baseDelayMs
        +
        leftLfo
        *
        modulationAmountMs;


    const float rightDelayMs =
        baseDelayMs
        -
        rightLfo
        *
        modulationAmountMs;


    // ======================================================
    // CONVERT DELAY TO SAMPLES
    // ======================================================

    const float leftDelaySamples =
        juce::jlimit(
            1.0f,
            static_cast<float>(
                delayBufferSize - 2
            ),
            leftDelayMs
            *
            static_cast<float>(
                sampleRate / 1000.0
            )
        );


    const float rightDelaySamples =
        juce::jlimit(
            1.0f,
            static_cast<float>(
                delayBufferSize - 2
            ),
            rightDelayMs
            *
            static_cast<float>(
                sampleRate / 1000.0
            )
        );


    // ======================================================
    // READ POSITIONS
    // ======================================================

    const float leftReadPosition =
        wrapPosition(
            static_cast<float>(
                writePosition
            )
            -
            leftDelaySamples,
            static_cast<float>(
                delayBufferSize
            )
        );


    const float rightReadPosition =
        wrapPosition(
            static_cast<float>(
                writePosition
            )
            -
            rightDelaySamples,
            static_cast<float>(
                delayBufferSize
            )
        );


    // ======================================================
    // INTERPOLATED DELAY READ
    // ======================================================

    const float delayedLeft =
        readInterpolated(
            0,
            leftReadPosition
        );


    const float delayedRight =
        readInterpolated(
            numChannels > 1 ? 1 : 0,
            rightReadPosition
        );


    // ======================================================
    // STEREO WIDTH
    // ======================================================

    const float widthAmount =
        currentWidth;


    const float center =
        0.5f;


    const float leftGain =
        center
        +
        center
        *
        widthAmount;


    const float rightGain =
        center
        +
        center
        *
        widthAmount;


    // ======================================================
    // CROSS-FEED
    // ======================================================

    const float leftDoubled =
        delayedLeft
        *
        leftGain
        +
        delayedRight
        *
        (
            1.0f
            -
            widthAmount
        )
        *
        0.5f;


    const float rightDoubled =
        delayedRight
        *
        rightGain
        +
        delayedLeft
        *
        (
            1.0f
            -
            widthAmount
        )
        *
        0.5f;


    // ======================================================
    // AMOUNT
    // ======================================================

    const float amountValue =
        currentAmount;


    const float doubledLeft =
        leftDoubled
        *
        amountValue;


    const float doubledRight =
        rightDoubled
        *
        amountValue;


    // ======================================================
    // MIX
    // ======================================================

    const float mixAmount =
        currentMix;


    const float dryGain =
        1.0f
        -
        mixAmount;


    const float wetGain =
        mixAmount;


    const float outputLeft =
        inputLeft
        *
        dryGain
        +
        doubledLeft
        *
        wetGain;


    const float outputRight =
        inputRight
        *
        dryGain
        +
        doubledRight
        *
        wetGain;


    // ======================================================
    // OUTPUT
    // ======================================================

    if (buffer.getNumChannels() >= 1)
    {
        buffer.setSample(
            0,
            sampleIndex,
            outputLeft
        );
    }


    if (buffer.getNumChannels() >= 2)
    {
        buffer.setSample(
            1,
            sampleIndex,
            outputRight
        );
    }


    // ======================================================
    // ADVANCE DELAY POSITION
    // ======================================================

    ++writePosition;


    if (writePosition >= delayBufferSize)
        writePosition = 0;
}


// ==========================================================
// INTERPOLATED DELAY READ
// ==========================================================

float DoublerProcessor::readInterpolated(
    int channel,
    float position) const
{
    if (channel < 0 ||
        channel >= delayBuffer.getNumChannels())
    {
        return 0.0f;
    }


    const float bufferSize =
        static_cast<float>(
            delayBuffer.getNumSamples()
        );


    position =
        wrapPosition(
            position,
            bufferSize
        );


    const int indexA =
        static_cast<int>(
            std::floor(position)
        );


    const int indexB =
        (
            indexA + 1
        )
        %
        delayBuffer.getNumSamples();


    const float fraction =
        position
        -
        static_cast<float>(
            indexA
        );


    const float sampleA =
        delayBuffer.getSample(
            channel,
            indexA
        );


    const float sampleB =
        delayBuffer.getSample(
            channel,
            indexB
        );


    return
        sampleA
        +
        (
            sampleB
            -
            sampleA
        )
        *
        fraction;
}


// ==========================================================
// WRAP POSITION
// ==========================================================

float DoublerProcessor::wrapPosition(
    float position,
    float bufferSize)
{
    while (position < 0.0f)
        position += bufferSize;


    while (position >= bufferSize)
        position -= bufferSize;


    return position;
}


// ==========================================================
// SINE LFO
// ==========================================================

float DoublerProcessor::sineLfo(
    double phase)
{
    return static_cast<float>(
        std::sin(
            phase
            *
            juce::MathConstants<double>::twoPi
        )
    );
}