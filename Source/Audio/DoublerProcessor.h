#pragma once

#include <JuceHeader.h>

class DoublerProcessor
{
public:

    DoublerProcessor();


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
    // QUALITY / CPU SETTINGS
    // ==========================================================

    ProcessingQuality processingQuality =
        ProcessingQuality::High;

    CPUMode cpuMode =
        CPUMode::Balanced;


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
    // LFO QUALITY STATE
    //
    // Instead of calculating sin() for every sample at every
    // quality level, lower quality modes calculate the LFO at
    // controlled intervals and interpolate between the values.
    //
    // This is important:
    //
    // LOW / MEDIUM do NOT simply hold the LFO value.
    //
    // Holding the value would create staircase modulation and
    // could produce audible artifacts.
    //
    // We calculate two points and smoothly interpolate between
    // them.
    // ==========================================================

    float leftLfoCurrent = 0.0f;
    float leftLfoTarget = 0.0f;

    float rightLfoCurrent = 0.0f;
    float rightLfoTarget = 0.0f;


    int leftLfoSamplesRemaining = 0;
    int rightLfoSamplesRemaining = 0;

    int leftLfoUpdateInterval = 1;
    int rightLfoUpdateInterval = 1;


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
    // QUALITY HELPERS
    // ==========================================================

    int getBaseLfoUpdateInterval() const;

    int getEffectiveLfoUpdateInterval() const;


    // ==========================================================
    // LFO PROCESSING
    // ==========================================================

    float getNextLeftLfoValue();

    float getNextRightLfoValue();


    // ==========================================================
    // AUDIO PROCESSING
    // ==========================================================

    void processSample(
        juce::AudioBuffer<float>& buffer,
        int sampleIndex
    );


    // ==========================================================
    // DELAY READ
    // ==========================================================

    float readInterpolated(
        int channel,
        float position
    ) const;


    // ==========================================================
    // HELPERS
    // ==========================================================

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