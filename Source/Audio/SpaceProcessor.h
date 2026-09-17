#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

class SpaceProcessor
{
public:

    enum class Type
    {
        Off = 0,
        Room,
        Plate,
        Hall,
        Dark,
        Air,
        Dream
    };

    SpaceProcessor();

    void prepare(
        double sampleRate,
        int maximumBlockSize,
        int numChannels
    );

    void reset();

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        int type,
        float size,
        float decay,
        float preDelay,
        float damping,
        float mix
    );

private:

    double sampleRate = 44100.0;
    int maximumBlockSize = 512;
    int numChannels = 2;

    bool prepared = false;

    juce::dsp::Reverb reverb;

    juce::AudioBuffer<float> dryBuffer;

    juce::HeapBlock<float> delayBuffer;
    int delayBufferSize = 0;
    int delayWritePosition = 0;

    float currentSize = 0.5f;
    float targetSize = 0.5f;

    float currentDecay = 0.5f;
    float targetDecay = 0.5f;

    float currentDamping = 0.5f;
    float targetDamping = 0.5f;

    float currentMix = 0.0f;
    float targetMix = 0.0f;

    float currentPreDelay = 0.0f;
    float targetPreDelay = 0.0f;

    static constexpr float smoothingCoefficient = 0.015f;

    void updateParameters(
        int type,
        float size,
        float decay,
        float preDelay,
        float damping
    );

    float readDelay(
        int channel,
        float delaySamples
    ) const;

    void writeDelay(
        int channel,
        float sample
    );

    static float equalPowerDry(float mix);
    static float equalPowerWet(float mix);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        SpaceProcessor
    );
};