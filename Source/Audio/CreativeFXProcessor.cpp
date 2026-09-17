#include "CreativeFXProcessor.h"

#include <cmath>

CreativeFXProcessor::CreativeFXProcessor()
{
}

void CreativeFXProcessor::prepare(
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

    delayBuffer.setSize(
        numChannels,
        delayBufferSize
    );

    delayBuffer.clear();

    juce::dsp::ProcessSpec spec;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize =
        static_cast<juce::uint32>(
            maximumBlockSize
        );
    spec.numChannels =
        static_cast<juce::uint32>(
            numChannels
        );

    highPassFilterLeft.reset();
    highPassFilterRight.reset();

    lowPassFilterLeft.reset();
    lowPassFilterRight.reset();

    highPassFilterLeft.prepare(spec);
    highPassFilterRight.prepare(spec);

    lowPassFilterLeft.prepare(spec);
    lowPassFilterRight.prepare(spec);

    highPassFilterLeft.setType(
        juce::dsp::StateVariableTPTFilter<float>::Type::highpass
    );

    highPassFilterRight.setType(
        juce::dsp::StateVariableTPTFilter<float>::Type::highpass
    );

    lowPassFilterLeft.setType(
        juce::dsp::StateVariableTPTFilter<float>::Type::lowpass
    );

    lowPassFilterRight.setType(
        juce::dsp::StateVariableTPTFilter<float>::Type::lowpass
    );

    reset();

    prepared = true;
}

void CreativeFXProcessor::reset()
{
    delayBuffer.clear();

    delayWritePosition = 0;

    envelope = 0.0f;

    currentAmount = 0.0f;
    targetAmount = 0.0f;

    currentMix = 0.0f;
    targetMix = 0.0f;

    robotPhase = 0.0;

    highPassFilterLeft.reset();
    highPassFilterRight.reset();

    lowPassFilterLeft.reset();
    lowPassFilterRight.reset();
}

void CreativeFXProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    int type,
    float amount,
    float mix
)
{
    if (!prepared ||
        buffer.getNumSamples() <= 0 ||
        buffer.getNumChannels() <= 0)
    {
        return;
    }

    amount =
        juce::jlimit(
            0.0f,
            1.0f,
            amount
        );

    mix =
        juce::jlimit(
            0.0f,
            1.0f,
            mix
        );

    targetAmount = amount;
    targetMix = mix;

    const float parameterSmoothing = 0.0025f;

    for (int sample = 0;
         sample < buffer.getNumSamples();
         ++sample)
    {
        currentAmount +=
            (targetAmount - currentAmount)
            * parameterSmoothing;

        currentMix +=
            (targetMix - currentMix)
            * parameterSmoothing;

        const float leftInput =
            buffer.getSample(
                0,
                sample
            );

        const float rightInput =
            buffer.getNumChannels() > 1
                ? buffer.getSample(1, sample)
                : leftInput;

        const float monoInput =
            0.5f *
            (leftInput + rightInput);

        const float currentEnvelope =
            calculateEnvelope(
                monoInput
            );

        envelope = currentEnvelope;

        const Type fxType =
            static_cast<Type>(
                juce::jlimit(
                    0,
                    static_cast<int>(Type::Wide),
                    type
                )
            );

        float wetLeft =
            processSample(
                leftInput,
                0,
                fxType,
                currentAmount,
                leftInput,
                rightInput
            );

        float wetRight =
            processSample(
                rightInput,
                1,
                fxType,
                currentAmount,
                leftInput,
                rightInput
            );

        //------------------------------------------------------
        // Equal-power dry/wet
        //------------------------------------------------------

        const float dryGain =
            equalPowerDry(currentMix);

        const float wetGain =
            equalPowerWet(currentMix);

        buffer.setSample(
            0,
            sample,
            leftInput * dryGain
            + wetLeft * wetGain
        );

        if (buffer.getNumChannels() > 1)
        {
            buffer.setSample(
                1,
                sample,
                rightInput * dryGain
                + wetRight * wetGain
            );
        }

        //------------------------------------------------------
        // Delay memory
        //------------------------------------------------------

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

        delayWritePosition++;

        if (delayWritePosition >= delayBufferSize)
            delayWritePosition = 0;
    }
}

float CreativeFXProcessor::processSample(
    float input,
    int channel,
    Type type,
    float amount,
    float leftInput,
    float rightInput
)
{
    switch (type)
    {
        case Type::Off:
            return input;

        //======================================================
        // TELEPHONE
        //======================================================

        case Type::Telephone:
        {
            const float highPass =
                450.0f
                + amount * 700.0f;

            const float lowPass =
                3200.0f
                - amount * 800.0f;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    highPass
                );

                lowPassFilterLeft.setCutoffFrequency(
                    lowPass
                );

                float sample = input;

                sample =
                    highPassFilterLeft.processSample(
                        0,
                        sample
                    );

                sample =
                    lowPassFilterLeft.processSample(
                        0,
                        sample
                    );

                sample =
                    softClip(
                        sample * (1.5f + amount * 2.5f)
                    );

                return sample;
            }
            else
            {
                highPassFilterRight.setCutoffFrequency(
                    highPass
                );

                lowPassFilterRight.setCutoffFrequency(
                    lowPass
                );

                float sample = input;

                sample =
                    highPassFilterRight.processSample(
                        0,
                        sample
                    );

                sample =
                    lowPassFilterRight.processSample(
                        0,
                        sample
                    );

                sample =
                    softClip(
                        sample * (1.5f + amount * 2.5f)
                    );

                return sample;
            }
        }

        //======================================================
        // RADIO
        //======================================================

        case Type::Radio:
        {
            const float highPass =
                180.0f + amount * 250.0f;

            const float lowPass =
                5200.0f - amount * 1800.0f;

            float sample = input;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    highPass
                );

                lowPassFilterLeft.setCutoffFrequency(
                    lowPass
                );

                sample =
                    highPassFilterLeft.processSample(
                        0,
                        sample
                    );

                sample =
                    lowPassFilterLeft.processSample(
                        0,
                        sample
                    );
            }
            else
            {
                highPassFilterRight.setCutoffFrequency(
                    highPass
                );

                lowPassFilterRight.setCutoffFrequency(
                    lowPass
                );

                sample =
                    highPassFilterRight.processSample(
                        0,
                        sample
                    );

                sample =
                    lowPassFilterRight.processSample(
                        0,
                        sample
                    );
            }

            return softClip(
                sample *
                (1.3f + amount * 2.0f)
            );
        }

        //======================================================
        // MEGAPHONE
        //======================================================

        case Type::Megaphone:
        {
            const float highPass =
                650.0f + amount * 500.0f;

            const float lowPass =
                2600.0f - amount * 700.0f;

            float sample = input;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    highPass
                );

                lowPassFilterLeft.setCutoffFrequency(
                    lowPass
                );

                sample =
                    highPassFilterLeft.processSample(
                        0,
                        sample
                    );

                sample =
                    lowPassFilterLeft.processSample(
                        0,
                        sample
                    );
            }
            else
            {
                highPassFilterRight.setCutoffFrequency(
                    highPass
                );

                lowPassFilterRight.setCutoffFrequency(
                    lowPass
                );

                sample =
                    highPassFilterRight.processSample(
                        0,
                        sample
                    );

                sample =
                    lowPassFilterRight.processSample(
                        0,
                        sample
                    );
            }

            return softClip(
                sample *
                (2.0f + amount * 3.0f)
            );
        }

        //======================================================
        // LO-FI
        //======================================================

        case Type::LoFi:
        {
            const float crushed =
                bitCrush(
                    input,
                    amount
                );

            return softClip(
                crushed *
                (1.0f + amount * 1.5f)
            );
        }

        //======================================================
        // WHISPER
        //======================================================

        case Type::Whisper:
        {
            const float highPass =
                900.0f + amount * 800.0f;

            float sample = input;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    highPass
                );

                sample =
                    highPassFilterLeft.processSample(
                        0,
                        sample
                    );
            }
            else
            {
                highPassFilterRight.setCutoffFrequency(
                    highPass
                );

                sample =
                    highPassFilterRight.processSample(
                        0,
                        sample
                    );
            }

            const float breath =
                std::tanh(
                    std::abs(sample) *
                    (2.0f + amount * 8.0f)
                );

            return
                (sample * 0.35f)
                + (breath * 0.15f);
        }

        //======================================================
        // ROBOT
        //======================================================

        case Type::Robot:
        {
            const double frequency =
                65.0
                + static_cast<double>(
                    envelope
                ) * 110.0;

            robotPhase +=
                frequency / sampleRate;

            if (robotPhase >= 1.0)
                robotPhase -= 1.0;

            const float carrier =
                std::sin(
                    static_cast<float>(
                        robotPhase *
                        juce::MathConstants<double>::twoPi
                    )
                );

            return
                input *
                (1.0f - amount * 0.55f)
                +
                carrier *
                std::abs(input) *
                amount *
                0.45f;
        }

        //======================================================
        // DARK
        //======================================================

        case Type::Dark:
        {
            const float lowPass =
                5000.0f
                - amount * 3500.0f;

            if (channel == 0)
            {
                lowPassFilterLeft.setCutoffFrequency(
                    lowPass
                );

                return
                    lowPassFilterLeft.processSample(
                        0,
                        input
                    );
            }

            lowPassFilterRight.setCutoffFrequency(
                lowPass
            );

            return
                lowPassFilterRight.processSample(
                    0,
                    input
                );
        }

        //======================================================
        // BRIGHT
        //======================================================

        case Type::Bright:
        {
            const float highPass =
                180.0f
                + amount * 250.0f;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    highPass
                );

                return
                    highPassFilterLeft.processSample(
                        0,
                        input
                    ) * (1.0f + amount * 0.35f);
            }

            highPassFilterRight.setCutoffFrequency(
                highPass
            );

            return
                highPassFilterRight.processSample(
                    0,
                    input
                ) * (1.0f + amount * 0.35f);
        }

        //======================================================
        // DISTORTED
        //======================================================

        case Type::Distorted:
        {
            const float drive =
                1.0f
                + amount * 14.0f;

            return softClip(
                input * drive
            );
        }

        //======================================================
        // DREAM
        //======================================================

        case Type::Dream:
        {
            const float delayed =
                readDelay(
                    channel,
                    800.0f
                    + amount * 1800.0f
                );

            return
                input * 0.70f
                +
                delayed * amount * 0.65f;
        }

        //======================================================
        // WIDE
        //======================================================

        case Type::Wide:
        {
            const float side =
                (leftInput - rightInput)
                * (0.5f + amount);

            if (channel == 0)
                return leftInput + side;

            return rightInput - side;
        }
    }

    return input;
}

float CreativeFXProcessor::readDelay(
    int channel,
    float delaySamples
) const
{
    float position =
        static_cast<float>(
            delayWritePosition
        )
        - delaySamples;

    while (position < 0.0f)
        position +=
            static_cast<float>(
                delayBufferSize
            );

    while (position >=
           static_cast<float>(delayBufferSize))
    {
        position -=
            static_cast<float>(
                delayBufferSize
            );
    }

    const int indexA =
        static_cast<int>(position);

    const int indexB =
        (indexA + 1) %
        delayBufferSize;

    const float fraction =
        position -
        static_cast<float>(indexA);

    const float sampleA =
        delayBuffer.getSample(
            channel,
            indexA
        );

    const float sampleB =
        delayBuffer.getSample(
            channel,
            indexB
        );

    return
        sampleA +
        (sampleB - sampleA)
        * fraction;
}

void CreativeFXProcessor::writeDelay(
    int channel,
    float sample
)
{
    if (channel >= delayBuffer.getNumChannels())
        return;

    delayBuffer.setSample(
        channel,
        delayWritePosition,
        sample
    );
}

float CreativeFXProcessor::softClip(
    float sample
) const
{
    return std::tanh(sample);
}

float CreativeFXProcessor::bitCrush(
    float sample,
    float amount
) const
{
    const float bits =
        16.0f
        - amount * 10.0f;

    const float levels =
        std::pow(
            2.0f,
            juce::jmax(
                2.0f,
                bits
            )
        );

    return
        std::round(
            sample * levels
        )
        / levels;
}

float CreativeFXProcessor::calculateEnvelope(
    float input
)
{
    const float absolute =
        std::abs(input);

    if (absolute > envelope)
    {
        envelope +=
            (absolute - envelope)
            * 0.15f;
    }
    else
    {
        envelope *= 0.995f;
    }

    return envelope;
}

float CreativeFXProcessor::equalPowerDry(
    float mix
)
{
    return std::cos(
        mix *
        juce::MathConstants<float>::halfPi
    );
}

float CreativeFXProcessor::equalPowerWet(
    float mix
)
{
    return std::sin(
        mix *
        juce::MathConstants<float>::halfPi
    );
}