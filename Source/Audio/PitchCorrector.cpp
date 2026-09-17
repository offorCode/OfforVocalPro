#include "PitchCorrector.h"

#include <cmath>

// ==========================================================
// CONSTRUCTOR
// ==========================================================

PitchCorrector::PitchCorrector()
{
}

// ==========================================================
// PREPARE
// ==========================================================

void PitchCorrector::prepare(
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

    window.allocate(
        grainSize,
        true
    );

    for (int i = 0;
         i < grainSize;
         ++i)
    {
        const float phase =
            static_cast<float>(i)
            /
            static_cast<float>(grainSize - 1);

        window[i] =
            0.5f
            -
            0.5f
            *
            std::cos(
                2.0f
                *
                juce::MathConstants<float>::pi
                *
                phase
            );
    }

    reset();

    prepared = true;
}

// ==========================================================
// RESET
// ==========================================================

void PitchCorrector::reset()
{
    if (delayBuffer.getNumChannels() > 0)
        delayBuffer.clear();

    writePosition = 0;

    grainCounter = 0;

    currentRatio = 1.0f;

    targetRatio = 1.0f;

    correctionMix = 0.0f;
    targetCorrectionMix = 0.0f;

    inputWasSilent = true;

    inputEnvelope = 0.0f;

    startupSamplesRemaining =
        startupWarmupSamples;

    resetGrains();
}

// ==========================================================
// RESET GRAINS
// ==========================================================

void PitchCorrector::resetGrains()
{
    grainCounter = 0;

    for (auto& grain : grains)
    {
        grain.active = false;
        grain.readPosition = 0.0f;
        grain.phase = 0.0f;
        grain.increment = 1.0f;
    }
}

// ==========================================================
// PROCESS BLOCK
// ==========================================================

void PitchCorrector::processBlock(
    juce::AudioBuffer<float>& buffer,
    float detectedFrequency,
    float targetMidiNote,
    float strength,
    float retuneAmount,
    bool pitchDetected)
{
    if (!prepared)
        return;

    const int numSamples =
        buffer.getNumSamples();

    const int bufferChannels =
        buffer.getNumChannels();

    if (numSamples <= 0 ||
        bufferChannels <= 0)
    {
        return;
    }

    strength =
        juce::jlimit(
            0.0f,
            1.0f,
            strength
        );

    retuneAmount =
        juce::jlimit(
            0.0f,
            1.0f,
            retuneAmount
        );

    // ======================================================
    // SMOOTH CORRECTION ON/OFF
    // ======================================================

    targetCorrectionMix =
        (pitchDetected &&
        detectedFrequency > 20.0f)
            ? strength
            : 0.0f;

    const float fadeTimeSamples =
        static_cast<float>(
            juce::jmax(
                1.0,
                sampleRate *
                (correctionFadeTimeMs / 1000.0)
            )
        );

    const float fadeCoefficient =
        1.0f -
        std::exp(
            -1.0f / fadeTimeSamples
        );

    // ======================================================
    // TARGET RATIO
    // ======================================================

    float desiredRatio = 1.0f;

    if (pitchDetected &&
        detectedFrequency > 20.0f &&
        targetMidiNote >= 0.0f)
    {
        desiredRatio =
            calculateTargetRatio(
                detectedFrequency,
                targetMidiNote
            );
    }

    desiredRatio =
        juce::jlimit(
            0.5f,
            2.0f,
            desiredRatio
        );

    targetRatio = desiredRatio;

    // ======================================================
    // RETUNE
    // ======================================================

    const float smoothing =
        0.0005f
        +
        retuneAmount
        *
        0.008f;

    const float ratioSmoothing =
        juce::jlimit(
            0.0001f,
            0.05f,
            smoothing
        );

    // ======================================================
    // PROCESS
    // ======================================================

    for (int sample = 0;
         sample < numSamples;
         ++sample)
    {
        currentRatio +=
            (
                targetRatio
                -
                currentRatio
            )
            *
            ratioSmoothing;

        currentRatio =
            juce::jlimit(
                0.5f,
                2.0f,
                currentRatio
            );
        
        // Smoothly enter/leave pitch correction.
        correctionMix +=
            (
                targetCorrectionMix
                -
                correctionMix
            )
            *
            fadeCoefficient;

        processSample(
            buffer,
            sample,
            correctionMix
        );
    }
}

// ==========================================================
// PROCESS SAMPLE
// ==========================================================

void PitchCorrector::processSample(
    juce::AudioBuffer<float>& buffer,
    int sampleIndex,
    float correctionAmount)
{
    const int bufferChannels =
        buffer.getNumChannels();

    // ======================================================
    // SAVE ORIGINAL INPUT
    // ======================================================

    float original[2] =
    {
        0.0f,
        0.0f
    };

    for (int channel = 0;
         channel < bufferChannels &&
         channel < 2;
         ++channel)
    {
        original[channel] =
            buffer.getSample(
                channel,
                sampleIndex
            );
    }

    // ======================================================
    // INPUT LEVEL
    // ======================================================

    float inputLevel = 0.0f;

    for (int channel = 0;
         channel < bufferChannels &&
         channel < 2;
         ++channel)
    {
        inputLevel =
            juce::jmax(
                inputLevel,
                std::abs(
                    original[channel]
                )
            );
    }

    // ======================================================
    // ENVELOPE
    // ======================================================

    if (inputLevel > inputEnvelope)
    {
        inputEnvelope +=
            (
                inputLevel
                -
                inputEnvelope
            )
            *
            envelopeAttack;
    }
    else
    {
        inputEnvelope *=
            envelopeRelease;
    }

    const bool currentlySilent =
        inputEnvelope < silenceThreshold;

    // ======================================================
    // NEW NOTE AFTER SILENCE
    // ======================================================

    if (inputWasSilent &&
        !currentlySilent)
    {
        resetGrains();

        currentRatio = 1.0f;

        targetRatio = 1.0f;

        startupSamplesRemaining =
            startupWarmupSamples;

        correctionMix = 0.0f;
    }

    inputWasSilent =
        currentlySilent;

    // ======================================================
    // WRITE INPUT TO DELAY BUFFER
    // ======================================================

    for (int channel = 0;
         channel < numChannels;
         ++channel)
    {
        const int sourceChannel =
            juce::jmin(
                channel,
                bufferChannels - 1
            );

        delayBuffer.setSample(
            channel,
            writePosition,
            buffer.getSample(
                sourceChannel,
                sampleIndex
            )
        );
    }

    // ======================================================
    // WARMUP
    // ======================================================

    if (startupSamplesRemaining > 0)
    {
        --startupSamplesRemaining;

        ++writePosition;

        if (writePosition >= delayBufferSize)
            writePosition = 0;

        for (int channel = 0;
             channel < bufferChannels &&
             channel < 2;
             ++channel)
        {
            buffer.setSample(
                channel,
                sampleIndex,
                original[channel]
            );
        }

        return;
    }

    // ======================================================
    // START GRAIN
    // ======================================================

    if (grainCounter <= 0)
    {
        const int grainIndex =
            (writePosition / grainHop)
            %
            numGrains;

        Grain& grain =
            grains[grainIndex];

        float startPosition =
            static_cast<float>(
                writePosition
            )
            -
            minimumDelaySamples;

        startPosition =
            wrapPosition(
                startPosition,
                static_cast<float>(
                    delayBufferSize
                )
            );

        startGrain(
            grain,
            startPosition,
            currentRatio
        );

        grainCounter =
            grainHop;
    }

    --grainCounter;

    // ======================================================
    // RENDER
    // ======================================================

    float corrected[2] =
    {
        0.0f,
        0.0f
    };

    bool hasActiveGrain = false;

    for (int grainIndex = 0;
         grainIndex < numGrains;
         ++grainIndex)
    {
        Grain& grain =
            grains[grainIndex];

        if (!grain.active)
            continue;

        hasActiveGrain = true;

        const float windowValue =
            getWindow(
                grain.phase
            );

        if (windowValue > 0.0f)
        {
            for (int channel = 0;
                 channel < numChannels &&
                 channel < 2;
                 ++channel)
            {
                corrected[channel] +=
                    readInterpolated(
                        channel,
                        grain.readPosition
                    )
                    *
                    windowValue;
            }
        }

        grain.readPosition +=
            grain.increment;

        grain.readPosition =
            wrapPosition(
                grain.readPosition,
                static_cast<float>(
                    delayBufferSize
                )
            );

        grain.phase +=
            1.0f
            /
            static_cast<float>(
                grainSize
            );

        if (grain.phase >= 1.0f)
        {
            grain.phase = 1.0f;
            grain.active = false;
        }
    }

    // ======================================================
    // FALLBACK
    // ======================================================

    if (!hasActiveGrain)
    {
        const float fallbackPosition =
            wrapPosition(
                static_cast<float>(
                    writePosition
                )
                -
                minimumDelaySamples,
                static_cast<float>(
                    delayBufferSize
                )
            );

        for (int channel = 0;
             channel < numChannels &&
             channel < 2;
             ++channel)
        {
            corrected[channel] =
                readInterpolated(
                    channel,
                    fallbackPosition
                );
        }
    }

    // ======================================================
    // SAFE OUTPUT BLEND
    // ======================================================

    const float wet =
        juce::jlimit(
            0.0f,
            1.0f,
            correctionAmount
        );

    const float dry =
        1.0f - wet;

    for (int channel = 0;
         channel < bufferChannels &&
         channel < 2;
         ++channel)
    {
        float output =
            original[channel] * dry
            +
            corrected[channel] * wet;

        if (!std::isfinite(output))
            output = original[channel];

        output =
            juce::jlimit(
                -4.0f,
                4.0f,
                output
            );

        buffer.setSample(
            channel,
            sampleIndex,
            output
        );
    }

    // ======================================================
    // ADVANCE WRITE POSITION
    // ======================================================

    ++writePosition;

    if (writePosition >= delayBufferSize)
        writePosition = 0;
}

// ==========================================================
// START GRAIN
// ==========================================================

void PitchCorrector::startGrain(
    Grain& grain,
    float readPosition,
    float increment)
{
    grain.active = true;

    grain.readPosition =
        wrapPosition(
            readPosition,
            static_cast<float>(
                delayBufferSize
            )
        );

    grain.phase = 0.0f;

    grain.increment =
        juce::jlimit(
            0.5f,
            2.0f,
            increment
        );
}

// ==========================================================
// INTERPOLATED READ
// ==========================================================

float PitchCorrector::readInterpolated(
    int channel,
    float position) const
{
    if (channel < 0 ||
        channel >= delayBuffer.getNumChannels())
    {
        return 0.0f;
    }

    position =
        wrapPosition(
            position,
            static_cast<float>(
                delayBufferSize
            )
        );

    const int index1 =
        static_cast<int>(
            position
        );

    const int index2 =
        (
            index1 + 1
        )
        %
        delayBufferSize;

    const float fraction =
        position
        -
        static_cast<float>(
            index1
        );

    const float sample1 =
        delayBuffer.getSample(
            channel,
            index1
        );

    const float sample2 =
        delayBuffer.getSample(
            channel,
            index2
        );

    return
        sample1
        +
        (
            sample2
            -
            sample1
        )
        *
        fraction;
}

// ==========================================================
// WINDOW
// ==========================================================

float PitchCorrector::getWindow(
    float phase) const
{
    phase =
        juce::jlimit(
            0.0f,
            1.0f,
            phase
        );

    const float position =
        phase
        *
        static_cast<float>(
            grainSize - 1
        );

    const int index1 =
        static_cast<int>(
            position
        );

    const int index2 =
        juce::jmin(
            index1 + 1,
            grainSize - 1
        );

    const float fraction =
        position
        -
        static_cast<float>(
            index1
        );

    const float value1 =
        window[index1];

    const float value2 =
        window[index2];

    return
        value1
        +
        (
            value2
            -
            value1
        )
        *
        fraction;
}

// ==========================================================
// CALCULATE TARGET RATIO
// ==========================================================

float PitchCorrector::calculateTargetRatio(
    float detectedFrequency,
    float targetMidiNote) const
{
    if (detectedFrequency <= 0.0f)
        return 1.0f;

    const float targetFrequency =
        440.0f
        *
        std::pow(
            2.0f,
            (
                targetMidiNote
                -
                69.0f
            )
            /
            12.0f
        );

    if (!std::isfinite(targetFrequency) ||
        targetFrequency <= 0.0f)
    {
        return 1.0f;
    }

    float ratio =
        targetFrequency
        /
        detectedFrequency;

    if (!std::isfinite(ratio))
        ratio = 1.0f;

    return
        juce::jlimit(
            0.5f,
            2.0f,
            ratio
        );
}

// ==========================================================
// WRAP POSITION
// ==========================================================

float PitchCorrector::wrapPosition(
    float position,
    float bufferSize)
{
    if (bufferSize <= 0.0f)
        return 0.0f;

    while (position < 0.0f)
        position += bufferSize;

    while (position >= bufferSize)
        position -= bufferSize;

    return position;
}
