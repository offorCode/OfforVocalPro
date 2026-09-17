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


    delayBuffer.setSize(
        numChannels,
        delayBufferSize
    );

    delayBuffer.clear();


    prepared = true;

    reset();
}


// ==========================================================
// RESET
// ==========================================================

void DoublerProcessor::reset()
{
    clearDelayBuffer();

    writePosition = 0;

    lfoPhaseLeft = 0.0;
    lfoPhaseRight = 0.0;


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
            (targetAmount - currentAmount)
            * smoothing;

        currentDetune +=
            (targetDetune - currentDetune)
            * smoothing;

        currentTiming +=
            (targetTiming - currentTiming)
            * smoothing;

        currentWidth +=
            (targetWidth - currentWidth)
            * smoothing;

        currentMix +=
            (targetMix - currentMix)
            * smoothing;


        processSample(
            buffer,
            sample
        );
    }


    // ======================================================
    // UPDATE LFO PHASES
    // ======================================================

    const double leftIncrement =
        lfoRateLeft / sampleRate;

    const double rightIncrement =
        lfoRateRight / sampleRate;

    lfoPhaseLeft +=
        leftIncrement * samples;

    lfoPhaseRight +=
        rightIncrement * samples;


    lfoPhaseLeft =
        std::fmod(
            lfoPhaseLeft,
            1.0
        );

    lfoPhaseRight =
        std::fmod(
            lfoPhaseRight,
            1.0
        );
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
        inputLeft =
            buffer.getSample(
                0,
                sampleIndex
            );


    if (buffer.getNumChannels() >= 2)
        inputRight =
            buffer.getSample(
                1,
                sampleIndex
            );
    else
        inputRight = inputLeft;


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
        * currentTiming;


    // ======================================================
    // DETUNE
    //
    // Maximum modulation is deliberately small.
    //
    // This is what creates the natural "two takes"
    // impression rather than an obvious chorus.
    // ======================================================

    const float modulationAmountMs =
        maximumModulationMs
        * currentDetune
        * currentAmount;


    const float leftLfo =
        sineLfo(
            lfoPhaseLeft
        );


    const float rightLfo =
        sineLfo(
            lfoPhaseRight
        );


    // Slightly different modulation directions.
    const float leftDelayMs =
        baseDelayMs
        +
        leftLfo * modulationAmountMs;


    const float rightDelayMs =
        baseDelayMs
        -
        rightLfo * modulationAmountMs;


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
            * static_cast<float>(
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
            * static_cast<float>(
                sampleRate / 1000.0
            )
        );


    // ======================================================
    // READ DELAYED SIGNAL
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

    // At 0% width, both voices stay near the center.
    //
    // At 100%, the doubled voices are spread apart.

    const float widthAmount =
        currentWidth;


    const float center =
        0.5f;


    const float leftGain =
        center
        +
        center * widthAmount;


    const float rightGain =
        center
        +
        center * widthAmount;


    // Cross-feed keeps the effect usable at lower widths.

    const float leftDoubled =
        delayedLeft * leftGain
        +
        delayedRight
        * (1.0f - widthAmount)
        * 0.5f;


    const float rightDoubled =
        delayedRight * rightGain
        +
        delayedLeft
        * (1.0f - widthAmount)
        * 0.5f;


    // ======================================================
    // AMOUNT
    // ======================================================

    const float amount =
        currentAmount;


    const float doubledLeft =
        leftDoubled * amount;


    const float doubledRight =
        rightDoubled * amount;


    // ======================================================
    // MIX
    // ======================================================

    const float mixAmount =
        currentMix;


    const float dryGain =
        1.0f - mixAmount;


    const float wetGain =
        mixAmount;


    const float outputLeft =
        inputLeft * dryGain
        +
        doubledLeft * wetGain;


    const float outputRight =
        inputRight * dryGain
        +
        doubledRight * wetGain;


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
        (indexA + 1)
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
            sampleB - sampleA
        )
        * fraction;
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
            * juce::MathConstants<double>::twoPi
        )
    );
}