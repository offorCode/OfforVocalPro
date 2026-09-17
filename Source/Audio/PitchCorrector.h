#pragma once

#include <JuceHeader.h>

class PitchCorrector
{
public:

    PitchCorrector();

    // ==========================================================
    // PREPARE
    // ==========================================================

    void prepare(
        double sampleRate,
        int maximumBlockSize,
        int numChannels
    );

    // ==========================================================
    // RESET
    // ==========================================================

    void reset();

    // ==========================================================
    // RESPONSE CONTROL
    //
    // 0.0 = slower / smoother correction
    // 1.0 = faster / more aggressive correction
    //
    // This does NOT change the actual audio latency.
    // ==========================================================

    void setResponse(float newResponse);

    float getResponse() const
    {
        return response;
    }

    // ==========================================================
    // LATENCY
    //
    // The pitch engine intentionally looks behind the live
    // signal. We compensate the dry signal by the same amount.
    //
    // The DAW should also be told about this latency.
    // ==========================================================

    int getLatencySamples() const
    {
        return static_cast<int>(minimumDelaySamples);
    }

    double getLatencyMilliseconds() const
    {
        if (sampleRate <= 0.0)
            return 0.0;

        return
            (static_cast<double>(minimumDelaySamples)
             / sampleRate)
            * 1000.0;
    }

    // ==========================================================
    // PROCESS BLOCK
    // ==========================================================

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        float detectedFrequency,
        float targetMidiNote,
        float strength,
        float retuneAmount,
        bool pitchDetected
    );

private:

    // ==========================================================
    // AUDIO SETTINGS
    // ==========================================================

    double sampleRate = 44100.0;

    int maximumBlockSize = 512;

    int numChannels = 2;

    bool prepared = false;

    // ==========================================================
    // DELAY BUFFER
    //
    // This is the history buffer used by the granular
    // pitch-correction engine.
    // ==========================================================

    static constexpr int delayBufferSize = 65536;

    juce::AudioBuffer<float> delayBuffer;

    int writePosition = 0;

    // ==========================================================
    // DRY LATENCY COMPENSATION BUFFER
    //
    // IMPORTANT:
    //
    // The corrected signal comes from approximately
    // minimumDelaySamples in the past.
    //
    // Previously the dry signal was NOT delayed, causing:
    //
    //     DRY      = immediate
    //     CORRECTED = delayed
    //
    // which produced the audible slap-back / echo.
    //
    // This buffer delays the dry signal by exactly the same
    // amount as the pitch-corrected signal.
    // ==========================================================

    juce::AudioBuffer<float> dryDelayBuffer;

    // ==========================================================
    // GRAINS
    // ==========================================================

    static constexpr int grainSize = 4096;

    static constexpr int grainHop = 2048;

    static constexpr int numGrains = 2;

    // ==========================================================
    // PITCH ENGINE LATENCY
    //
    // 4096 samples at 44.1 kHz =
    // approximately 92.88 ms.
    //
    // 4096 samples at 48 kHz =
    // approximately 85.33 ms.
    //
    // This latency is compensated in the dry path.
    // ==========================================================

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
    // CORRECTION MIX
    // ==========================================================

    float correctionMix = 0.0f;

    float targetCorrectionMix = 0.0f;

    static constexpr float correctionFadeTimeMs = 20.0f;

    // ==========================================================
    // RESPONSE
    //
    // 0.0 = smooth / slow
    // 1.0 = fast / aggressive
    //
    // This controls how quickly currentRatio follows
    // targetRatio.
    // ==========================================================

    float response = 0.50f;

    float targetResponse = 0.50f;

    // ==========================================================
    // ONSET / SILENCE STATE
    // ==========================================================

    bool inputWasSilent = true;

    float inputEnvelope = 0.0f;

    static constexpr float silenceThreshold = 0.0015f;

    static constexpr float envelopeAttack = 0.20f;

    static constexpr float envelopeRelease = 0.995f;

    // ==========================================================
    // STARTUP
    //
    // Allows the delay buffer to fill with real audio before
    // granular processing begins.
    // ==========================================================

    int startupSamplesRemaining = 0;

    static constexpr int startupWarmupSamples = 8192;

    // ==========================================================
    // WINDOW
    // ==========================================================

    juce::HeapBlock<float> window;

    // ==========================================================
    // INTERNAL PROCESSING
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

    float readDryDelayed(
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