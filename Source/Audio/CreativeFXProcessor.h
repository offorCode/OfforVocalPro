#pragma once

#include <JuceHeader.h>

//==============================================================================
// OFFOR VOCAL PRO
// Creative FX Processor
//
// Processing Quality and CPU Mode are REAL DSP controls.
//
// Quality controls how often expensive/control-rate parameters are recalculated:
//
//     Low     = every 8 samples
//     Medium  = every 4 samples
//     High    = every 2 samples
//     Ultra   = every sample
//
// CPU Mode modifies that interval:
//
//     Low CPU      = 2x interval
//     Balanced     = normal interval
//     Performance  = 0.5x interval
//
// Audio itself is still processed sample-by-sample.
//==============================================================================

class CreativeFXProcessor
{
public:

    //==========================================================================

    enum class Type
    {
        Off,
        Telephone,
        Radio,
        Megaphone,
        LoFi,
        Whisper,
        Robot,
        Dark,
        Bright,
        Distorted,
        Wide,
        Dream
    };

    //==========================================================================
    // Processing Quality
    //==========================================================================

    enum class ProcessingQuality
    {
        Low,
        Medium,
        High,
        Ultra
    };

    //==========================================================================
    // CPU Mode
    //==========================================================================

    enum class CPUMode
    {
        LowCPU,
        Balanced,
        Performance
    };

    //==========================================================================

    CreativeFXProcessor();

    //==========================================================================

    void prepare(
        double sampleRate,
        int maximumBlockSize,
        int numChannels
    );

    void reset();

    //==========================================================================

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        Type type,
        float amount,
        float mix
    );

    //==========================================================================
    // REAL PROCESSING QUALITY / CPU SETTINGS
    //==========================================================================

    void setProcessingQuality(
        ProcessingQuality newQuality
    );

    void setCPUMode(
        CPUMode newMode
    );

private:

    //==========================================================================

    double sampleRate = 44100.0;
    int maximumBlockSize = 512;
    int numChannels = 2;

    bool prepared = false;

    //==========================================================================

    ProcessingQuality processingQuality =
        ProcessingQuality::High;

    CPUMode cpuMode =
        CPUMode::Balanced;

    // Number of samples between control-rate updates.
    //
    // IMPORTANT:
    // Audio processing itself still happens every sample.
    // This only controls how often filter/control values are recalculated.
    int controlUpdateInterval = 2;

    int samplesUntilControlUpdate = 0;

    //==========================================================================

    // Filters
    //==========================================================================

    juce::dsp::StateVariableTPTFilter<float> highPassFilterLeft;
    juce::dsp::StateVariableTPTFilter<float> highPassFilterRight;

    juce::dsp::StateVariableTPTFilter<float> lowPassFilterLeft;
    juce::dsp::StateVariableTPTFilter<float> lowPassFilterRight;

    //==========================================================================

    // Delay
    //==========================================================================

    static constexpr int delayBufferSize = 65536;

    juce::AudioBuffer<float> delayBuffer;

    int delayWritePosition = 0;

    //==========================================================================

    // Envelope
    //==========================================================================

    float envelope = 0.0f;

    //==========================================================================

    // Smoothed parameters
    //==========================================================================

    float currentAmount = 0.0f;
    float targetAmount = 0.0f;

    float currentMix = 0.0f;
    float targetMix = 0.0f;

    //==========================================================================

    // Cached control values
    //
    // These allow Quality/CPU Mode to reduce the number of expensive
    // parameter calculations without reducing sample-accurate audio output.
    //==========================================================================

    float cachedHighPassFrequency = 1000.0f;
    float cachedLowPassFrequency = 5000.0f;

    float cachedRobotFrequency = 65.0f;

    float cachedDrive = 1.0f;

    //==========================================================================

    // Robot oscillator
    //==========================================================================

    double robotPhase = 0.0;

    //==========================================================================

    // Helpers
    //==========================================================================

    void updateControlValues(
        Type type,
        float amount
    );

    void updateControlInterval();

    int getQualityInterval() const;

    float processSample(
        float input,
        int channel,
        Type type,
        float amount,
        float leftInput,
        float rightInput
    );

    float readDelay(
        int channel,
        float delaySamples
    ) const;

    void writeDelay(
        int channel,
        float sample
    );

    float softClip(
        float sample
    ) const;

    float bitCrush(
        float sample,
        float amount
    ) const;

    float calculateEnvelope(
        float input
    );

    static float equalPowerDry(
        float mix
    );

    static float equalPowerWet(
        float mix
    );

    //==========================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        CreativeFXProcessor
    );
};