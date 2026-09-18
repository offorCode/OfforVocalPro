#pragma once

#include <JuceHeader.h>

class HarmonyProcessor
{
public:

    HarmonyProcessor();


    // ==========================================================
    // PROCESSING QUALITY
    // ==========================================================

    enum class ProcessingQuality
    {
        Low,
        Medium,
        High,
        Ultra
    };


    // ==========================================================
    // CPU MODE
    // ==========================================================

    enum class CPUMode
    {
        LowCPU,
        Balanced,
        Performance
    };


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
    // PROCESSING QUALITY / CPU MODE
    // ==========================================================

    void setProcessingQuality(
        ProcessingQuality newQuality
    );

    void setCPUMode(
        CPUMode newMode
    );


    // ==========================================================
    // AUDIO PROCESSING
    // ==========================================================

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

    // ==========================================================
    // CONFIGURATION
    // ==========================================================

    double sampleRate = 44100.0;

    int maximumBlockSize = 512;

    int numChannels = 2;

    bool prepared = false;


    // ==========================================================
    // QUALITY / CPU SETTINGS
    // ==========================================================

    ProcessingQuality processingQuality =
        ProcessingQuality::High;

    CPUMode cpuMode =
        CPUMode::Balanced;


    // ==========================================================
    // DELAY / GRANULAR PITCH-SHIFTING
    // ==========================================================

    static constexpr int delayBufferSize = 65536;

    juce::AudioBuffer<float> delayBuffer;

    int writePosition = 0;


    static constexpr int grainSize = 4096;

    static constexpr int grainHop = 2048;

    static constexpr int numGrains = 2;

    static constexpr float minimumDelaySamples = 4096.0f;


    // ==========================================================
    // GRAIN
    // ==========================================================

    struct Grain
    {
        bool active = false;

        float readPosition = 0.0f;

        float phase = 0.0f;

        float increment = 1.0f;


        // ------------------------------------------------------
        // QUALITY-AWARE WINDOW STATE
        //
        // At High/Ultra the window can be calculated every
        // sample.
        //
        // At lower quality levels we calculate two window
        // points and smoothly interpolate between them.
        //
        // This reduces expensive cos() calls without creating
        // a staircase envelope.
        // ------------------------------------------------------

        float windowCurrent = 0.0f;

        float windowTarget = 0.0f;

        int windowSamplesRemaining = 0;
    };


    Grain grains[4][numGrains];

    int grainCounter[4] {};


    // ==========================================================
    // SMOOTHED PARAMETERS
    // ==========================================================

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


    // ==========================================================
    // PITCH TRACKING
    // ==========================================================

    float currentDetectedFrequency = 0.0f;

    float targetDetectedFrequency = 0.0f;

    bool currentPitchDetected = false;


    // ==========================================================
    // VOICE SETTINGS
    // ==========================================================

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


    // ==========================================================
    // QUALITY HELPERS
    // ==========================================================

    int getBaseWindowUpdateInterval() const;

    int getEffectiveWindowUpdateInterval() const;


    // ==========================================================
    // QUALITY-AWARE GRAIN WINDOW
    // ==========================================================

    float getQualityWindow(
        Grain& grain
    );


    // ==========================================================
    // INTERNAL PROCESSING
    // ==========================================================

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


    // ==========================================================
    // INTERPOLATED DELAY READ
    // ==========================================================

    float readInterpolated(
        int channel,
        float position
    ) const;


    // ==========================================================
    // ORIGINAL WINDOW CALCULATION
    // ==========================================================

    float getWindow(
        float phase
    ) const;


    // ==========================================================
    // PITCH RATIO
    // ==========================================================

    float calculatePitchRatio(
        float intervalSemitones
    ) const;


    // ==========================================================
    // HELPERS
    // ==========================================================

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