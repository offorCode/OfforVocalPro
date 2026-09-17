#pragma once

#include <JuceHeader.h>

class PitchCorrector
{
public:

    PitchCorrector();

    void prepare(
        double sampleRate,
        int maximumBlockSize,
        int numChannels
    );

    void reset();

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        float detectedFrequency,
        float targetMidiNote,
        float strength,
        float retuneAmount,
        bool pitchDetected
    );

private:

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
    // GRAINS
    // ==========================================================

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

    Grain grains[numGrains];

    int grainCounter = 0;

    // ==========================================================
    // PITCH
    // ==========================================================

    float currentRatio = 1.0f;
    float targetRatio = 1.0f;

    // ==========================================================
    // CORRECTION FADE
    // ==========================================================

    float correctionMix = 0.0f;
    float targetCorrectionMix = 0.0f;

    static constexpr float correctionFadeTimeMs = 20.0f;

    // ==========================================================
    // ONSET / SILENCE STATE
    // ==========================================================

    bool inputWasSilent = true;

    float inputEnvelope = 0.0f;

    static constexpr float silenceThreshold = 0.0015f;

    static constexpr float envelopeAttack = 0.20f;
    static constexpr float envelopeRelease = 0.995f;

    // Number of samples used to let the delay buffer
    // contain fresh audio before granular processing starts.
    int startupSamplesRemaining = 0;

    static constexpr int startupWarmupSamples = 8192;

    // ==========================================================
    // WINDOW
    // ==========================================================

    juce::HeapBlock<float> window;

    // ==========================================================
    // PROCESSING
    // ==========================================================

    void processSample(
        juce::AudioBuffer<float>& buffer,
        int sampleIndex,
        float correctionAmount
    );

    void startGrain(
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

    float calculateTargetRatio(
        float detectedFrequency,
        float targetMidiNote
    ) const;

    static float wrapPosition(
        float position,
        float bufferSize
    );

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        PitchCorrector
    );
};
