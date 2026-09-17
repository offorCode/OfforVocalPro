#include "SpaceProcessor.h"

#include <cmath>
#include <algorithm>

SpaceProcessor::SpaceProcessor()
{
}

void SpaceProcessor::prepare(
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

    const int maximumDelaySamples =
        static_cast<int>(
            sampleRate * 0.5
        );

    delayBufferSize =
        juce::jmax(
            1,
            maximumDelaySamples * numChannels
        );

    delayBuffer.allocate(
        static_cast<size_t>(delayBufferSize),
        true
    );

    dryBuffer.setSize(
        numChannels,
        maximumBlockSize
    );

    juce::dsp::ProcessSpec spec;

    spec.sampleRate =
        sampleRate;

    spec.maximumBlockSize =
        static_cast<juce::uint32>(
            maximumBlockSize
        );

    spec.numChannels =
        static_cast<juce::uint32>(
            numChannels
        );

    reverb.prepare(spec);

    juce::dsp::Reverb::Parameters parameters;

    parameters.roomSize = 0.5f;
    parameters.damping = 0.5f;
    parameters.wetLevel = 1.0f;
    parameters.dryLevel = 0.0f;
    parameters.width = 1.0f;
    parameters.freezeMode = 0.0f;

    reverb.setParameters(
        parameters
    );

    reset();

    prepared = true;
}

void SpaceProcessor::reset()
{
    reverb.reset();

    if (delayBuffer != nullptr &&
        delayBufferSize > 0)
    {
        juce::FloatVectorOperations::clear(
            delayBuffer.getData(),
            delayBufferSize
        );
    }

    dryBuffer.clear();

    delayWritePosition = 0;

    currentSize = 0.5f;
    targetSize = 0.5f;

    currentDecay = 0.5f;
    targetDecay = 0.5f;

    currentDamping = 0.5f;
    targetDamping = 0.5f;

    currentMix = 0.0f;
    targetMix = 0.0f;

    currentPreDelay = 0.0f;
    targetPreDelay = 0.0f;
}

void SpaceProcessor::updateParameters(
    int type,
    float size,
    float decay,
    float preDelay,
    float damping
)
{
    size =
        juce::jlimit(
            0.0f,
            1.0f,
            size
        );

    decay =
        juce::jlimit(
            0.0f,
            1.0f,
            decay
        );

    preDelay =
        juce::jlimit(
            0.0f,
            1.0f,
            preDelay
        );

    damping =
        juce::jlimit(
            0.0f,
            1.0f,
            damping
        );

    float roomSize = size;
    float reverbDecay = decay;
    float reverbDamping = damping;

    switch (static_cast<Type>(type))
    {
        case Type::Room:
        {
            roomSize =
                0.20f +
                size * 0.35f;

            reverbDecay =
                0.20f +
                decay * 0.35f;

            reverbDamping =
                0.45f +
                damping * 0.45f;

            break;
        }

        case Type::Plate:
        {
            roomSize =
                0.45f +
                size * 0.40f;

            reverbDecay =
                0.45f +
                decay * 0.50f;

            reverbDamping =
                0.20f +
                damping * 0.45f;

            break;
        }

        case Type::Hall:
        {
            roomSize =
                0.60f +
                size * 0.38f;

            reverbDecay =
                0.55f +
                decay * 0.42f;

            reverbDamping =
                0.20f +
                damping * 0.55f;

            break;
        }

        case Type::Dark:
        {
            roomSize =
                0.45f +
                size * 0.45f;

            reverbDecay =
                0.40f +
                decay * 0.50f;

            reverbDamping =
                0.70f +
                damping * 0.25f;

            break;
        }

        case Type::Air:
        {
            roomSize =
                0.50f +
                size * 0.45f;

            reverbDecay =
                0.35f +
                decay * 0.50f;

            reverbDamping =
                0.05f +
                damping * 0.25f;

            break;
        }

        case Type::Dream:
        {
            roomSize =
                0.70f +
                size * 0.29f;

            reverbDecay =
                0.65f +
                decay * 0.33f;

            reverbDamping =
                0.15f +
                damping * 0.35f;

            break;
        }

        case Type::Off:
        default:
        {
            break;
        }
    }

    targetSize =
        juce::jlimit(
            0.0f,
            1.0f,
            roomSize
        );

    targetDecay =
        juce::jlimit(
            0.0f,
            1.0f,
            reverbDecay
        );

    targetDamping =
        juce::jlimit(
            0.0f,
            1.0f,
            reverbDamping
        );

    targetPreDelay =
        preDelay;
}

float SpaceProcessor::readDelay(
    int channel,
    float delaySamples
) const
{
    if (delayBuffer == nullptr ||
        delayBufferSize <= 0)
    {
        return 0.0f;
    }

    const int channelLength =
        delayBufferSize /
        numChannels;

    if (channelLength <= 1)
        return 0.0f;

    delaySamples =
        juce::jlimit(
            0.0f,
            static_cast<float>(
                channelLength - 2
            ),
            delaySamples
        );

    float readPosition =
        static_cast<float>(
            delayWritePosition
        )
        - delaySamples;

    while (readPosition < 0.0f)
        readPosition +=
            static_cast<float>(
                channelLength
            );

    while (readPosition >=
           static_cast<float>(
               channelLength
           ))
    {
        readPosition -=
            static_cast<float>(
                channelLength
            );
    }

    const int indexA =
        static_cast<int>(
            readPosition
        );

    const int indexB =
        (indexA + 1)
        % channelLength;

    const float fraction =
        readPosition -
        static_cast<float>(
            indexA
        );

    const int base =
        channel *
        channelLength;

    const float sampleA =
        delayBuffer[
            base + indexA
        ];

    const float sampleB =
        delayBuffer[
            base + indexB
        ];

    return
        sampleA +
        (
            sampleB -
            sampleA
        ) *
        fraction;
}

void SpaceProcessor::writeDelay(
    int channel,
    float sample
)
{
    if (delayBuffer == nullptr ||
        delayBufferSize <= 0)
    {
        return;
    }

    const int channelLength =
        delayBufferSize /
        numChannels;

    if (channelLength <= 0)
        return;

    const int safeChannel =
        juce::jlimit(
            0,
            numChannels - 1,
            channel
        );

    const int index =
        safeChannel *
        channelLength +
        delayWritePosition;

    delayBuffer[index] =
        sample;
}

float SpaceProcessor::equalPowerDry(
    float mix
)
{
    mix =
        juce::jlimit(
            0.0f,
            1.0f,
            mix
        );

    return std::cos(
        mix *
        juce::MathConstants<float>::halfPi
    );
}

float SpaceProcessor::equalPowerWet(
    float mix
)
{
    mix =
        juce::jlimit(
            0.0f,
            1.0f,
            mix
        );

    return std::sin(
        mix *
        juce::MathConstants<float>::halfPi
    );
}

void SpaceProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    int type,
    float size,
    float decay,
    float preDelay,
    float damping,
    float mix
)
{
    if (!prepared ||
        buffer.getNumSamples() <= 0)
    {
        return;
    }

    const int channels =
        juce::jmin(
            buffer.getNumChannels(),
            numChannels
        );

    const int samples =
        buffer.getNumSamples();

    if (channels <= 0)
        return;

    type =
        juce::jlimit(
            0,
            6,
            type
        );

    size =
        juce::jlimit(
            0.0f,
            1.0f,
            size
        );

    decay =
        juce::jlimit(
            0.0f,
            1.0f,
            decay
        );

    preDelay =
        juce::jlimit(
            0.0f,
            1.0f,
            preDelay
        );

    damping =
        juce::jlimit(
            0.0f,
            1.0f,
            damping
        );

    mix =
        juce::jlimit(
            0.0f,
            1.0f,
            mix
        );

    if (type ==
        static_cast<int>(
            Type::Off
        ))
    {
        currentMix = 0.0f;
        targetMix = 0.0f;

        return;
    }

    updateParameters(
        type,
        size,
        decay,
        preDelay,
        damping
    );

    targetMix =
        mix;

    if (dryBuffer.getNumChannels() !=
            channels ||
        dryBuffer.getNumSamples() <
            samples)
    {
        dryBuffer.setSize(
            channels,
            juce::jmax(
                samples,
                maximumBlockSize
            ),
            false,
            false,
            true
        );
    }

    for (int channel = 0;
         channel < channels;
         ++channel)
    {
        dryBuffer.copyFrom(
            channel,
            0,
            buffer,
            channel,
            0,
            samples
        );
    }

    currentSize +=
        (
            targetSize -
            currentSize
        ) *
        smoothingCoefficient;

    currentDecay +=
        (
            targetDecay -
            currentDecay
        ) *
        smoothingCoefficient;

    currentDamping +=
        (
            targetDamping -
            currentDamping
        ) *
        smoothingCoefficient;

    currentPreDelay +=
        (
            targetPreDelay -
            currentPreDelay
        ) *
        smoothingCoefficient;

    currentMix +=
        (
            targetMix -
            currentMix
        ) *
        smoothingCoefficient;

    /*
        JUCE's built-in Reverb does not
        expose a separate decay-time
        parameter.

        Therefore the Vocal Pro Decay
        control influences the effective
        room size, which changes the tail
        length.

        Size still controls the space
        dimensions, while Decay pushes
        the tail shorter or longer.
    */
    const float effectiveRoomSize =
        juce::jlimit(
            0.0f,
            1.0f,
            currentSize * 0.72f +
            currentDecay * 0.28f
        );

    juce::dsp::Reverb::Parameters parameters;

    parameters.roomSize =
        effectiveRoomSize;

    parameters.damping =
        currentDamping;

    parameters.wetLevel =
        1.0f;

    parameters.dryLevel =
        0.0f;

    parameters.width =
        1.0f;

    parameters.freezeMode =
        0.0f;

    reverb.setParameters(
        parameters
    );

    constexpr float maximumPreDelayMs =
        120.0f;

    const float preDelayMs =
        currentPreDelay *
        maximumPreDelayMs;

    const float preDelaySamples =
        static_cast<float>(
            sampleRate *
            preDelayMs /
            1000.0
        );

    /*
        First create the pre-delayed
        wet signal directly inside
        buffer.
    */
    const int channelLength =
        delayBufferSize /
        numChannels;

    for (int sample = 0;
         sample < samples;
         ++sample)
    {
        const float leftInput =
            dryBuffer.getSample(
                0,
                sample
            );

        const float rightInput =
            channels > 1
            ? dryBuffer.getSample(
                1,
                sample
            )
            : leftInput;

        writeDelay(
            0,
            leftInput
        );

        if (numChannels > 1)
        {
            writeDelay(
                1,
                rightInput
            );
        }

        const float delayedLeft =
            readDelay(
                0,
                preDelaySamples
            );

        const float delayedRight =
            numChannels > 1
            ? readDelay(
                1,
                preDelaySamples
            )
            : delayedLeft;

        buffer.setSample(
            0,
            sample,
            delayedLeft
        );

        if (channels > 1)
        {
            buffer.setSample(
                1,
                sample,
                delayedRight
            );
        }

        ++delayWritePosition;

        if (delayWritePosition >=
            channelLength)
        {
            delayWritePosition = 0;
        }
    }

    /*
        Process the complete stereo block
        through JUCE 8's DSP API.

        This replaces the old
        reverb.processStereo() call.
    */
    juce::dsp::AudioBlock<float>
        audioBlock(buffer);

    auto processBlock =
        audioBlock.getSubsetChannelBlock(
            0,
            static_cast<size_t>(
                channels
            )
        );

    juce::dsp::ProcessContextReplacing<float>
        context(processBlock);

    reverb.process(
        context
    );

    /*
        Blend original vocal and
        reverb using equal-power mixing.
    */
    const float dryGain =
        equalPowerDry(
            currentMix
        );

    const float wetGain =
        equalPowerWet(
            currentMix
        );

    for (int channel = 0;
         channel < channels;
         ++channel)
    {
        const float* dry =
            dryBuffer.getReadPointer(
                channel
            );

        float* wet =
            buffer.getWritePointer(
                channel
            );

        for (int sample = 0;
             sample < samples;
             ++sample)
        {
            wet[sample] =
                dry[sample] *
                dryGain +
                wet[sample] *
                wetGain;
        }
    }
}