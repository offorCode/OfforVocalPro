#pragma once

#include <JuceHeader.h>

class HarmonyProcessor
{
public:

    HarmonyProcessor();

    void prepare(
        double sampleRate,
        int maximumBlockSize,
        int numChannels
    );

    void reset();

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        float voice1Interval,
        float voice2Interval,
        float voice3Interval,
        float voice4Interval,
        float harmonyMix,
        float detectedFrequency,
        bool pitchDetected
    );

private:

    double sampleRate = 44100.0;
    int maximumBlockSize = 512;
    int numChannels = 2;

    bool prepared = false;

    // ------------------------------------------------------------
    // Delay / granular pitch-shifting
    // ------------------------------------------------------------

    static constexpr int delayBufferSize = 65536;

    juce::AudioBuffer<float> delayBuffer;

    int writePosition = 0;

    static constexpr int grainSize = 4096;
    static constexpr int grainHop = 2048;
    static constexpr int numGrains = 2;

    static constexpr float minimumDelaySamples = 4096.0f;

    struct Grain
    {
        bool active = false;
        float readPosition = 0.0f;
        float phase = 0.0f;
        float increment = 1.0f;
    };

    Grain grains[4][numGrains];

    int grainCounter[4] {};

    // ------------------------------------------------------------
    // Smoothed parameters
    // ------------------------------------------------------------

    float currentVoice1Interval = 0.0f;
    float targetVoice1Interval = 0.0f;

    float currentVoice2Interval = 0.0f;
    float targetVoice2Interval = 0.0f;

    float currentVoice3Interval = 0.0f;
    float targetVoice3Interval = 0.0f;

    float currentVoice4Interval = 0.0f;
    float targetVoice4Interval = 0.0f;

    float currentMix = 0.0f;
    float targetMix = 0.0f;

    // ------------------------------------------------------------
    // Pitch tracking
    // ------------------------------------------------------------

    float currentDetectedFrequency = 0.0f;
    float targetDetectedFrequency = 0.0f;

    bool currentPitchDetected = false;

    // ------------------------------------------------------------
    // Voice settings
    // ------------------------------------------------------------

    static constexpr float voice1Level = 0.72f;
    static constexpr float voice2Level = 0.58f;
    static constexpr float voice3Level = 0.48f;
    static constexpr float voice4Level = 0.40f;

    static constexpr float voice1Pan = -0.55f;
    static constexpr float voice2Pan = 0.55f;
    static constexpr float voice3Pan = -0.85f;
    static constexpr float voice4Pan = 0.85f;

    // Small natural detune offsets.
    static constexpr float voice1DetuneCents = -4.0f;
    static constexpr float voice2DetuneCents = 4.0f;
    static constexpr float voice3DetuneCents = -7.0f;
    static constexpr float voice4DetuneCents = 7.0f;

    // ------------------------------------------------------------
    // Internal processing
    // ------------------------------------------------------------

    void processSample(
        juce::AudioBuffer<float>& buffer,
        int sampleIndex,
        const float voiceIntervals[4],
        float harmonyMix
    );

    void processVoiceSample(
        int voiceIndex,
        float intervalSemitones,
        float harmonyMix,
        float inputLeft,
        float inputRight,
        float& outputLeft,
        float& outputRight
    );

    void startGrain(
        int voiceIndex,
        Grain& grain,
        float readPosition,
        float increment
    );

    void resetGrains();

    float readInterpolated(
        int channel,
        float position
    ) const;

    float getWindow(
        float phase
    ) const;

    float calculatePitchRatio(
        float intervalSemitones
    ) const;

    static float wrapPosition(
        float position,
        float bufferSize
    );

    static float centsToRatio(
        float cents
    );

    static float dbToGain(
        float db
    );

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        HarmonyProcessor
    );
};