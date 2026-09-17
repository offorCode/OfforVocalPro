#pragma once

#include <JuceHeader.h>

class DoublerProcessor
{
public:

    DoublerProcessor();

    void prepare(
        double sampleRate,
        int maximumBlockSize,
        int numChannels
    );

    void reset();

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        float amount,
        float detune,
        float timing,
        float width,
        float mix
    );

private:

    // ==========================================================
    // CONFIGURATION
    // ==========================================================

    double sampleRate = 44100.0;

    int maximumBlockSize = 512;

    int numChannels = 2;

    bool prepared = false;


    // ==========================================================
    // DELAY BUFFER
    // ==========================================================

    static constexpr int delayBufferSize = 65536;

    juce::AudioBuffer<float> delayBuffer;

    int writePosition = 0;


    // ==========================================================
    // LFO STATE
    // ==========================================================

    double lfoPhaseLeft = 0.0;

    double lfoPhaseRight = 0.0;


    // ==========================================================
    // SMOOTHED PARAMETERS
    // ==========================================================

    float currentAmount = 0.0f;
    float targetAmount = 0.0f;

    float currentDetune = 0.0f;
    float targetDetune = 0.0f;

    float currentTiming = 0.0f;
    float targetTiming = 0.0f;

    float currentWidth = 0.0f;
    float targetWidth = 0.0f;

    float currentMix = 0.0f;
    float targetMix = 0.0f;


    // ==========================================================
    // LFO
    // ==========================================================

    static constexpr float lfoRateLeft = 0.17f;
    static constexpr float lfoRateRight = 0.23f;


    // ==========================================================
    // DELAY LIMITS
    // ==========================================================

    static constexpr float minimumDelayMs = 8.0f;
    static constexpr float maximumDelayMs = 35.0f;

    static constexpr float maximumModulationMs = 3.0f;


    // ==========================================================
    // HELPERS
    // ==========================================================

    void processSample(
        juce::AudioBuffer<float>& buffer,
        int sampleIndex
    );

    float readInterpolated(
        int channel,
        float position
    ) const;

    static float wrapPosition(
        float position,
        float bufferSize
    );

    static float sineLfo(
        double phase
    );

    void clearDelayBuffer();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        DoublerProcessor
    );
};