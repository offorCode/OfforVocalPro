#include "CreativeFXProcessor.h"

#include <cmath>

//==============================================================================
// Constructor
//==============================================================================

CreativeFXProcessor::CreativeFXProcessor()
{
}

//==============================================================================
// PREPARE
//==============================================================================

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

    //==========================================================================

    delayBuffer.setSize(
        numChannels,
        delayBufferSize
    );

    delayBuffer.clear();

    //==========================================================================

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

    //==========================================================================

    highPassFilterLeft.reset();
    highPassFilterRight.reset();

    lowPassFilterLeft.reset();
    lowPassFilterRight.reset();

    //==========================================================================

    highPassFilterLeft.prepare(spec);
    highPassFilterRight.prepare(spec);

    lowPassFilterLeft.prepare(spec);
    lowPassFilterRight.prepare(spec);

    //==========================================================================

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

    //==========================================================================

    updateControlInterval();

    reset();

    prepared = true;
}

//==============================================================================
// RESET
//==============================================================================

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

    samplesUntilControlUpdate = 0;

    //==========================================================================

    cachedHighPassFrequency = 1000.0f;
    cachedLowPassFrequency = 5000.0f;

    cachedRobotFrequency = 65.0f;

    cachedDrive = 1.0f;

    //==========================================================================

    highPassFilterLeft.reset();
    highPassFilterRight.reset();

    lowPassFilterLeft.reset();
    lowPassFilterRight.reset();
}

//==============================================================================
// PROCESSING QUALITY
//==============================================================================

void CreativeFXProcessor::setProcessingQuality(
    ProcessingQuality newQuality
)
{
    processingQuality = newQuality;

    updateControlInterval();

    // Force an immediate control update.
    samplesUntilControlUpdate = 0;
}

//==============================================================================
// CPU MODE
//==============================================================================

void CreativeFXProcessor::setCPUMode(
    CPUMode newMode
)
{
    cpuMode = newMode;

    updateControlInterval();

    // Force an immediate control update.
    samplesUntilControlUpdate = 0;
}

//==============================================================================
// QUALITY BASE INTERVAL
//==============================================================================

int CreativeFXProcessor::getQualityInterval() const
{
    switch (processingQuality)
    {
        case ProcessingQuality::Low:
            return 8;

        case ProcessingQuality::Medium:
            return 4;

        case ProcessingQuality::High:
            return 2;

        case ProcessingQuality::Ultra:
            return 1;
    }

    return 2;
}

//==============================================================================
// UPDATE CONTROL INTERVAL
//==============================================================================

void CreativeFXProcessor::updateControlInterval()
{
    int interval = getQualityInterval();

    switch (cpuMode)
    {
        //======================================================================
        // LOW CPU
        //
        // Recalculate controls less frequently.
        //======================================================================

        case CPUMode::LowCPU:
            interval *= 2;
            break;

        //======================================================================
        // BALANCED
        //======================================================================

        case CPUMode::Balanced:
            break;

        //======================================================================
        // PERFORMANCE
        //
        // Recalculate controls more frequently.
        //======================================================================

        case CPUMode::Performance:
            interval = juce::jmax(
                1,
                interval / 2
            );
            break;
    }

    controlUpdateInterval =
        juce::jmax(
            1,
            interval
        );

    samplesUntilControlUpdate = 0;
}

//==============================================================================
// CONTROL VALUE UPDATE
//
// This function performs the calculations that do not need to happen on every
// audio sample.
//
// The filters themselves still process every sample.
//
// This is where Processing Quality and CPU Mode have real DSP consequences.
//==============================================================================

void CreativeFXProcessor::updateControlValues(
    Type type,
    float amount
)
{
    switch (type)
    {
        //======================================================================
        // TELEPHONE
        //======================================================================

        case Type::Telephone:

            cachedHighPassFrequency =
                450.0f
                + amount * 700.0f;

            cachedLowPassFrequency =
                3200.0f
                - amount * 800.0f;

            cachedDrive =
                1.5f
                + amount * 2.5f;

            break;

        //======================================================================
        // RADIO
        //======================================================================

        case Type::Radio:

            cachedHighPassFrequency =
                180.0f
                + amount * 250.0f;

            cachedLowPassFrequency =
                5200.0f
                - amount * 1800.0f;

            cachedDrive =
                1.3f
                + amount * 2.0f;

            break;

        //======================================================================
        // MEGAPHONE
        //======================================================================

        case Type::Megaphone:

            cachedHighPassFrequency =
                650.0f
                + amount * 500.0f;

            cachedLowPassFrequency =
                2600.0f
                - amount * 700.0f;

            cachedDrive =
                2.0f
                + amount * 3.0f;

            break;

        //======================================================================
        // WHISPER
        //======================================================================

        case Type::Whisper:

            cachedHighPassFrequency =
                900.0f
                + amount * 800.0f;

            break;

        //======================================================================
        // ROBOT
        //======================================================================

        case Type::Robot:

            cachedRobotFrequency =
                65.0f
                + static_cast<double>(
                    envelope
                ) * 110.0f;

            break;

        //======================================================================
        // DARK
        //======================================================================

        case Type::Dark:

            cachedLowPassFrequency =
                5000.0f
                - amount * 3500.0f;

            break;

        //======================================================================
        // BRIGHT
        //======================================================================

        case Type::Bright:

            cachedHighPassFrequency =
                180.0f
                + amount * 250.0f;

            break;

        //======================================================================
        // DISTORTED
        //======================================================================

        case Type::Distorted:

            cachedDrive =
                1.0f
                + amount * 14.0f;

            break;

        //======================================================================
        // OTHER EFFECTS
        //======================================================================

        case Type::Off:
        case Type::LoFi:
        case Type::Wide:
        case Type::Dream:
            break;
    }
}

//==============================================================================
// PROCESS BLOCK
//==============================================================================

void CreativeFXProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    Type type,
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

    //==========================================================================

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

    //==========================================================================

    targetAmount = amount;
    targetMix = mix;

    const float parameterSmoothing = 0.0025f;

    //==========================================================================

    for (int sample = 0;
         sample < buffer.getNumSamples();
         ++sample)
    {
        //======================================================================
        // PARAMETER SMOOTHING
        //======================================================================

        currentAmount +=
            (targetAmount - currentAmount)
            * parameterSmoothing;

        currentMix +=
            (targetMix - currentMix)
            * parameterSmoothing;

        //======================================================================
        // INPUT
        //======================================================================

        const float leftInput =
            buffer.getSample(
                0,
                sample
            );

        const float rightInput =
            buffer.getNumChannels() > 1
                ? buffer.getSample(
                    1,
                    sample
                )
                : leftInput;

        //======================================================================
        // MONO ENVELOPE
        //======================================================================

        const float monoInput =
            0.5f *
            (leftInput + rightInput);

        const float currentEnvelope =
            calculateEnvelope(
                monoInput
            );

        envelope = currentEnvelope;

        //======================================================================
        // CONTROL-RATE UPDATE
        //
        // This is the REAL Quality/CPU optimization.
        //
        // Audio is still processed every sample, but expensive control values
        // are recalculated only at the selected interval.
        //======================================================================

        if (samplesUntilControlUpdate <= 0)
        {
            updateControlValues(
                type,
                currentAmount
            );

            samplesUntilControlUpdate =
                controlUpdateInterval;
        }

        --samplesUntilControlUpdate;

        //======================================================================
        // PROCESS EFFECT
        //======================================================================

        const Type fxType = type;

        const float wetLeft =
            processSample(
                leftInput,
                0,
                fxType,
                currentAmount,
                leftInput,
                rightInput
            );

        const float wetRight =
            processSample(
                rightInput,
                1,
                fxType,
                currentAmount,
                leftInput,
                rightInput
            );

        //======================================================================
        // EQUAL-POWER DRY / WET
        //======================================================================

        const float dryGain =
            equalPowerDry(
                currentMix
            );

        const float wetGain =
            equalPowerWet(
                currentMix
            );

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

        //======================================================================
        // DELAY MEMORY
        //======================================================================

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

//==============================================================================
// PROCESS SAMPLE
//==============================================================================

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
        //======================================================================
        // OFF
        //======================================================================

        case Type::Off:
            return input;

        //======================================================================
        // TELEPHONE
        //======================================================================

        case Type::Telephone:
        {
            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    cachedHighPassFrequency
                );

                lowPassFilterLeft.setCutoffFrequency(
                    cachedLowPassFrequency
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

                return softClip(
                    sample * cachedDrive
                );
            }

            highPassFilterRight.setCutoffFrequency(
                cachedHighPassFrequency
            );

            lowPassFilterRight.setCutoffFrequency(
                cachedLowPassFrequency
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

            return softClip(
                sample * cachedDrive
            );
        }

        //======================================================================
        // RADIO
        //======================================================================

        case Type::Radio:
        {
            float sample = input;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    cachedHighPassFrequency
                );

                lowPassFilterLeft.setCutoffFrequency(
                    cachedLowPassFrequency
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
                    cachedHighPassFrequency
                );

                lowPassFilterRight.setCutoffFrequency(
                    cachedLowPassFrequency
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
                sample * cachedDrive
            );
        }

        //======================================================================
        // MEGAPHONE
        //======================================================================

        case Type::Megaphone:
        {
            float sample = input;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    cachedHighPassFrequency
                );

                lowPassFilterLeft.setCutoffFrequency(
                    cachedLowPassFrequency
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
                    cachedHighPassFrequency
                );

                lowPassFilterRight.setCutoffFrequency(
                    cachedLowPassFrequency
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
                sample * cachedDrive
            );
        }

        //======================================================================
        // LO-FI
        //======================================================================

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

        //======================================================================
        // WHISPER
        //======================================================================

        case Type::Whisper:
        {
            float sample = input;

            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    cachedHighPassFrequency
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
                    cachedHighPassFrequency
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
                +
                (breath * 0.15f);
        }

        //======================================================================
        // ROBOT
        //======================================================================

        case Type::Robot:
        {
            //==============================================================
            // IMPORTANT:
            //
            // The oscillator phase is STILL updated every sample.
            //
            // Quality/CPU Mode only controls how often the envelope-driven
            // carrier frequency is recalculated.
            //==============================================================

            robotPhase +=
                cachedRobotFrequency /
                sampleRate;

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

        //======================================================================
        // DARK
        //======================================================================

        case Type::Dark:
        {
            if (channel == 0)
            {
                lowPassFilterLeft.setCutoffFrequency(
                    cachedLowPassFrequency
                );

                return
                    lowPassFilterLeft.processSample(
                        0,
                        input
                    );
            }

            lowPassFilterRight.setCutoffFrequency(
                cachedLowPassFrequency
            );

            return
                lowPassFilterRight.processSample(
                    0,
                    input
                );
        }

        //======================================================================
        // BRIGHT
        //======================================================================

        case Type::Bright:
        {
            if (channel == 0)
            {
                highPassFilterLeft.setCutoffFrequency(
                    cachedHighPassFrequency
                );

                return
                    highPassFilterLeft.processSample(
                        0,
                        input
                    )
                    *
                    (1.0f + amount * 0.35f);
            }

            highPassFilterRight.setCutoffFrequency(
                cachedHighPassFrequency
            );

            return
                highPassFilterRight.processSample(
                    0,
                    input
                )
                *
                (1.0f + amount * 0.35f);
        }

        //======================================================================
        // DISTORTED
        //======================================================================

        case Type::Distorted:
        {
            return softClip(
                input * cachedDrive
            );
        }

        //======================================================================
        // DREAM
        //======================================================================

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

        //======================================================================
        // WIDE
        //======================================================================

        case Type::Wide:
        {
            const float side =
                (leftInput - rightInput)
                *
                (0.5f + amount);

            if (channel == 0)
                return leftInput + side;

            return rightInput - side;
        }
    }

    return input;
}

//==============================================================================
// READ DELAY
//==============================================================================

float CreativeFXProcessor::readDelay(
    int channel,
    float delaySamples
) const
{
    float position =
        static_cast<float>(
            delayWritePosition
        )
        -
        delaySamples;

    while (position < 0.0f)
    {
        position +=
            static_cast<float>(
                delayBufferSize
            );
    }

    while (
        position >=
        static_cast<float>(
            delayBufferSize
        )
    )
    {
        position -=
            static_cast<float>(
                delayBufferSize
            );
    }

    const int indexA =
        static_cast<int>(
            position
        );

    const int indexB =
        (indexA + 1)
        %
        delayBufferSize;

    const float fraction =
        position -
        static_cast<float>(
            indexA
        );

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
        sampleA
        +
        (sampleB - sampleA)
        *
        fraction;
}

//==============================================================================
// WRITE DELAY
//==============================================================================

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

//==============================================================================
// SOFT CLIP
//==============================================================================

float CreativeFXProcessor::softClip(
    float sample
) const
{
    return std::tanh(sample);
}

//==============================================================================
// BIT CRUSH
//==============================================================================

float CreativeFXProcessor::bitCrush(
    float sample,
    float amount
) const
{
    const float bits =
        16.0f
        -
        amount * 10.0f;

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
        /
        levels;
}

//==============================================================================
// ENVELOPE
//==============================================================================

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
            *
            0.15f;
    }
    else
    {
        envelope *= 0.995f;
    }

    return envelope;
}

//==============================================================================
// EQUAL POWER DRY
//==============================================================================

float CreativeFXProcessor::equalPowerDry(
    float mix
)
{
    return std::cos(
        mix *
        juce::MathConstants<float>::halfPi
    );
}

//==============================================================================
// EQUAL POWER WET
//==============================================================================

float CreativeFXProcessor::equalPowerWet(
    float mix
)
{
    return std::sin(
        mix *
        juce::MathConstants<float>::halfPi
    );
}