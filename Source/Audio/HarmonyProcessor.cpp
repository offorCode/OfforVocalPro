#include "HarmonyProcessor.h"

#include <cmath>
#include <algorithm>


HarmonyProcessor::HarmonyProcessor()
{
}


//==============================================================
// PREPARE
//==============================================================

void HarmonyProcessor::prepare(
    double newSampleRate,
    int newMaximumBlockSize,
    int newNumChannels
)
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

    reset();

    prepared = true;
}


//==============================================================
// RESET
//==============================================================

void HarmonyProcessor::reset()
{
    delayBuffer.clear();

    writePosition = 0;

    resetGrains();

    currentVoice1Interval = 0.0f;
    targetVoice1Interval = 0.0f;

    currentVoice2Interval = 0.0f;
    targetVoice2Interval = 0.0f;

    currentVoice3Interval = 0.0f;
    targetVoice3Interval = 0.0f;

    currentVoice4Interval = 0.0f;
    targetVoice4Interval = 0.0f;

    currentMix = 0.0f;
    targetMix = 0.0f;

    currentDetectedFrequency = 0.0f;
    targetDetectedFrequency = 0.0f;

    currentPitchDetected = false;

    for (auto& counter : grainCounter)
        counter = 0;
}


//==============================================================
// PROCESS BLOCK
//==============================================================

void HarmonyProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    float voice1Interval,
    float voice2Interval,
    float voice3Interval,
    float voice4Interval,
    float harmonyMix,
    float detectedFrequency,
    bool pitchDetected
)
{
    if (!prepared ||
        buffer.getNumSamples() <= 0)
    {
        return;
    }

    voice1Interval =
        juce::jlimit(
            -24.0f,
            24.0f,
            voice1Interval
        );

    voice2Interval =
        juce::jlimit(
            -24.0f,
            24.0f,
            voice2Interval
        );

    voice3Interval =
        juce::jlimit(
            -24.0f,
            24.0f,
            voice3Interval
        );

    voice4Interval =
        juce::jlimit(
            -24.0f,
            24.0f,
            voice4Interval
        );

    harmonyMix =
        juce::jlimit(
            0.0f,
            1.0f,
            harmonyMix
        );

    if (!std::isfinite(detectedFrequency) ||
        detectedFrequency < 20.0f ||
        detectedFrequency > 2000.0f)
    {
        pitchDetected = false;
        detectedFrequency = 0.0f;
    }

    targetVoice1Interval = voice1Interval;
    targetVoice2Interval = voice2Interval;
    targetVoice3Interval = voice3Interval;
    targetVoice4Interval = voice4Interval;

    targetMix = harmonyMix;

    if (pitchDetected)
        targetDetectedFrequency = detectedFrequency;

    else
        targetDetectedFrequency = 0.0f;

    const float parameterSmoothing =
        0.015f;

    currentVoice1Interval +=
        (targetVoice1Interval -
         currentVoice1Interval)
        * parameterSmoothing;

    currentVoice2Interval +=
        (targetVoice2Interval -
         currentVoice2Interval)
        * parameterSmoothing;

    currentVoice3Interval +=
        (targetVoice3Interval -
         currentVoice3Interval)
        * parameterSmoothing;

    currentVoice4Interval +=
        (targetVoice4Interval -
         currentVoice4Interval)
        * parameterSmoothing;

    currentMix +=
        (targetMix -
         currentMix)
        * parameterSmoothing;

    currentDetectedFrequency +=
        (targetDetectedFrequency -
         currentDetectedFrequency)
        * 0.025f;

    currentPitchDetected = pitchDetected;

    const float voiceIntervals[4] =
    {
        currentVoice1Interval,
        currentVoice2Interval,
        currentVoice3Interval,
        currentVoice4Interval
    };

    for (int sample = 0;
         sample < buffer.getNumSamples();
         ++sample)
    {
        processSample(
            buffer,
            sample,
            voiceIntervals,
            currentMix
        );
    }
}


//==============================================================
// PROCESS SAMPLE
//==============================================================

void HarmonyProcessor::processSample(
    juce::AudioBuffer<float>& buffer,
    int sampleIndex,
    const float voiceIntervals[4],
    float harmonyMix
)
{
    const int channels =
        buffer.getNumChannels();

    const float inputLeft =
        buffer.getSample(
            0,
            sampleIndex
        );

    const float inputRight =
        channels > 1
            ? buffer.getSample(
                1,
                sampleIndex
              )
            : inputLeft;

    // ----------------------------------------------------------
    // Write input into delay buffer
    // ----------------------------------------------------------

    delayBuffer.setSample(
        0,
        writePosition,
        inputLeft
    );

    if (numChannels > 1)
    {
        delayBuffer.setSample(
            1,
            writePosition,
            inputRight
        );
    }

    // ----------------------------------------------------------
    // No usable pitch
    // ----------------------------------------------------------

    if (!currentPitchDetected ||
        currentDetectedFrequency < 20.0f ||
        currentMix <= 0.0001f)
    {
        buffer.setSample(
            0,
            sampleIndex,
            inputLeft
        );

        if (channels > 1)
        {
            buffer.setSample(
                1,
                sampleIndex,
                inputRight
            );
        }

        writePosition =
            (writePosition + 1)
            % delayBufferSize;

        return;
    }

    // ----------------------------------------------------------
    // Generate harmony voices
    // ----------------------------------------------------------

    float harmonyLeft = 0.0f;
    float harmonyRight = 0.0f;

    for (int voice = 0;
         voice < 4;
         ++voice)
    {
        if (std::abs(voiceIntervals[voice]) < 0.01f)
            continue;

        float voiceLeft = 0.0f;
        float voiceRight = 0.0f;

        processVoiceSample(
            voice,
            voiceIntervals[voice],
            harmonyMix,
            inputLeft,
            inputRight,
            voiceLeft,
            voiceRight
        );

        harmonyLeft += voiceLeft;
        harmonyRight += voiceRight;
    }

    // ----------------------------------------------------------
    // Keep harmony controlled
    // ----------------------------------------------------------

    const float voiceCount =
        4.0f;

    const float normalization =
        1.0f /
        std::sqrt(voiceCount);

    harmonyLeft *= normalization;
    harmonyRight *= normalization;

    // ----------------------------------------------------------
    // Add harmony to original vocal
    // ----------------------------------------------------------

    const float dryAmount =
        1.0f - harmonyMix;

    const float outputLeft =
        inputLeft * dryAmount
        + harmonyLeft;

    const float outputRight =
        inputRight * dryAmount
        + harmonyRight;

    buffer.setSample(
        0,
        sampleIndex,
        outputLeft
    );

    if (channels > 1)
    {
        buffer.setSample(
            1,
            sampleIndex,
            outputRight
        );
    }

    // ----------------------------------------------------------
    // Advance delay buffer
    // ----------------------------------------------------------

    writePosition =
        (writePosition + 1)
        % delayBufferSize;
}


//==============================================================
// PROCESS VOICE
//==============================================================

void HarmonyProcessor::processVoiceSample(
    int voiceIndex,
    float intervalSemitones,
    float harmonyMix,
    float inputLeft,
    float inputRight,
    float& outputLeft,
    float& outputRight
)
{
    outputLeft = 0.0f;
    outputRight = 0.0f;

    if (voiceIndex < 0 ||
        voiceIndex >= 4)
    {
        return;
    }

    // ----------------------------------------------------------
    // Pitch ratio
    // ----------------------------------------------------------

    float ratio =
        calculatePitchRatio(
            intervalSemitones
        );

    // ----------------------------------------------------------
    // Small natural detuning per voice
    // ----------------------------------------------------------

    float detuneCents = 0.0f;

    switch (voiceIndex)
    {
        case 0:
            detuneCents =
                voice1DetuneCents;
            break;

        case 1:
            detuneCents =
                voice2DetuneCents;
            break;

        case 2:
            detuneCents =
                voice3DetuneCents;
            break;

        case 3:
            detuneCents =
                voice4DetuneCents;
            break;

        default:
            break;
    }

    ratio *=
        centsToRatio(
            detuneCents
        );

    ratio =
        juce::jlimit(
            0.5f,
            2.0f,
            ratio
        );

    // ----------------------------------------------------------
    // Start / update grains
    // ----------------------------------------------------------

    Grain& grainA =
        grains[voiceIndex][0];

    Grain& grainB =
        grains[voiceIndex][1];

    const float delay =
        minimumDelaySamples;

    if (!grainA.active)
    {
        startGrain(
            voiceIndex,
            grainA,
            wrapPosition(
                static_cast<float>(writePosition)
                - delay
                - static_cast<float>(grainSize),
                static_cast<float>(delayBufferSize)
            ),
            ratio
        );
    }

    if (!grainB.active)
    {
        startGrain(
            voiceIndex,
            grainB,
            wrapPosition(
                static_cast<float>(writePosition)
                - delay,
                static_cast<float>(delayBufferSize)
            ),
            ratio
        );
    }

    // ----------------------------------------------------------
    // Read grains
    // ----------------------------------------------------------

    float left = 0.0f;
    float right = 0.0f;

    int activeGrains = 0;

    for (int grainIndex = 0;
         grainIndex < numGrains;
         ++grainIndex)
    {
        Grain& grain =
            grains[voiceIndex][grainIndex];

        if (!grain.active)
            continue;

        const float windowValue =
            getWindow(grain.phase);

        const float readLeft =
            readInterpolated(
                0,
                grain.readPosition
            );

        const float readRight =
            numChannels > 1
                ? readInterpolated(
                    1,
                    grain.readPosition
                  )
                : readLeft;

        left +=
            readLeft *
            windowValue;

        right +=
            readRight *
            windowValue;

        grain.readPosition =
            wrapPosition(
                grain.readPosition +
                grain.increment,
                static_cast<float>(
                    delayBufferSize
                )
            );

        grain.phase +=
            1.0f /
            static_cast<float>(
                grainSize
            );

        if (grain.phase >= 1.0f)
        {
            grain.active = false;
            grain.phase = 0.0f;
        }

        ++activeGrains;
    }

    if (activeGrains > 0)
    {
        const float normalization =
            1.0f /
            static_cast<float>(
                activeGrains
            );

        left *= normalization;
        right *= normalization;
    }

    // ----------------------------------------------------------
    // Stereo pan
    // ----------------------------------------------------------

    float pan = 0.0f;

    switch (voiceIndex)
    {
        case 0:
            pan = voice1Pan;
            break;

        case 1:
            pan = voice2Pan;
            break;

        case 2:
            pan = voice3Pan;
            break;

        case 3:
            pan = voice4Pan;
            break;

        default:
            break;
    }

    pan =
        juce::jlimit(
            -1.0f,
            1.0f,
            pan
        );

    const float angle =
        (pan + 1.0f)
        * juce::MathConstants<float>::pi
        * 0.25f;

    const float leftGain =
        std::cos(angle);

    const float rightGain =
        std::sin(angle);

    const float monoVoice =
        0.5f *
        (left + right);

    left =
        monoVoice *
        leftGain;

    right =
        monoVoice *
        rightGain;

    // ----------------------------------------------------------
    // Voice level
    // ----------------------------------------------------------

    float level = 0.0f;

    switch (voiceIndex)
    {
        case 0:
            level = voice1Level;
            break;

        case 1:
            level = voice2Level;
            break;

        case 2:
            level = voice3Level;
            break;

        case 3:
            level = voice4Level;
            break;

        default:
            break;
    }

    const float gain =
        level *
        harmonyMix;

    outputLeft =
        left *
        gain;

    outputRight =
        right *
        gain;
}


//==============================================================
// START GRAIN
//==============================================================

void HarmonyProcessor::startGrain(
    int voiceIndex,
    Grain& grain,
    float readPosition,
    float increment
)
{
    if (voiceIndex < 0 ||
        voiceIndex >= 4)
    {
        return;
    }

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


//==============================================================
// RESET GRAINS
//==============================================================

void HarmonyProcessor::resetGrains()
{
    for (int voice = 0;
         voice < 4;
         ++voice)
    {
        for (int grain = 0;
             grain < numGrains;
             ++grain)
        {
            grains[voice][grain].active = false;
            grains[voice][grain].readPosition = 0.0f;
            grains[voice][grain].phase = 0.0f;
            grains[voice][grain].increment = 1.0f;
        }

        grainCounter[voice] = 0;
    }
}


//==============================================================
// INTERPOLATED READ
//==============================================================

float HarmonyProcessor::readInterpolated(
    int channel,
    float position
) const
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

    const int indexA =
        static_cast<int>(
            position
        );

    const int indexB =
        (indexA + 1)
        % delayBufferSize;

    const float fraction =
        position -
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
        sampleA +
        (sampleB - sampleA)
        * fraction;
}


//==============================================================
// WINDOW
//==============================================================

float HarmonyProcessor::getWindow(
    float phase
) const
{
    phase =
        juce::jlimit(
            0.0f,
            1.0f,
            phase
        );

    return
        0.5f -
        0.5f *
        std::cos(
            2.0f *
            juce::MathConstants<float>::pi *
            phase
        );
}


//==============================================================
// PITCH RATIO
//==============================================================

float HarmonyProcessor::calculatePitchRatio(
    float intervalSemitones
) const
{
    return std::pow(
        2.0f,
        intervalSemitones / 12.0f
    );
}


//==============================================================
// CENTS TO RATIO
//==============================================================

float HarmonyProcessor::centsToRatio(
    float cents
)
{
    return std::pow(
        2.0f,
        cents / 1200.0f
    );
}


//==============================================================
// dB TO GAIN
//==============================================================

float HarmonyProcessor::dbToGain(
    float db
)
{
    return std::pow(
        10.0f,
        db / 20.0f
    );
}


//==============================================================
// WRAP POSITION
//==============================================================

float HarmonyProcessor::wrapPosition(
    float position,
    float bufferSize
)
{
    while (position < 0.0f)
        position += bufferSize;

    while (position >= bufferSize)
        position -= bufferSize;

    return position;
}