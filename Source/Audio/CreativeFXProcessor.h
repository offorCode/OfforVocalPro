#pragma once

#include <JuceHeader.h>

class CreativeFXProcessor
{
public:

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

    CreativeFXProcessor();

    void prepare(
        double sampleRate,
        int maximumBlockSize,
        int numChannels
    );

    void reset();

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        Type type,
        float amount,
        float mix
    );

    

private:

    double sampleRate = 44100.0;
    int maximumBlockSize = 512;
    int numChannels = 2;

    bool prepared = false;

    //==========================================================
    // Filters
    //==========================================================

    juce::dsp::StateVariableTPTFilter<float> highPassFilterLeft;
    juce::dsp::StateVariableTPTFilter<float> highPassFilterRight;

    juce::dsp::StateVariableTPTFilter<float> lowPassFilterLeft;
    juce::dsp::StateVariableTPTFilter<float> lowPassFilterRight;

    //==========================================================
    // Delay
    //==========================================================

    static constexpr int delayBufferSize = 65536;

    juce::AudioBuffer<float> delayBuffer;

    int delayWritePosition = 0;

    //==========================================================
    // Envelope
    //==========================================================

    float envelope = 0.0f;

    //==========================================================
    // Smoothed parameters
    //==========================================================

    float currentAmount = 0.0f;
    float targetAmount = 0.0f;

    float currentMix = 0.0f;
    float targetMix = 0.0f;

    //==========================================================
    // Robot oscillator
    //==========================================================

    double robotPhase = 0.0;

    //==========================================================
    // Helpers
    //==========================================================

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        CreativeFXProcessor
    );
};