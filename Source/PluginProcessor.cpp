#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/ThemeManager.h"

#include <limits>


// ==========================================================
// LICENSE SESSION THREAD
// ==========================================================
//
// This replaces the old:
//
//     std::thread(...).detach();
//
// implementation.
//
// The thread is owned by OfforVocalProAudioProcessor.
//
// The processor destructor stops the thread before the
// processor's other members are destroyed.
//
// ==========================================================

OfforVocalProAudioProcessor::LicenseSessionThread::
LicenseSessionThread(
    OfforVocalProAudioProcessor& ownerProcessor)
    : juce::Thread("OFFOR Vocal Pro License"),
      owner(ownerProcessor)
{
}


// ==========================================================
// LICENSE SESSION THREAD - RUN
// ==========================================================

void
OfforVocalProAudioProcessor::LicenseSessionThread::
run()
{
    // ======================================================
    // REGISTER INSTALLATION
    // ======================================================
    //
    // This creates the installation on the server if it
    // does not already exist.
    //
    // If it already exists, the server updates the relevant
    // installation information.
    //
    // LicenseManager performs the synchronous HTTP request.
    //
    // This code is running on the background thread,
    // NOT the audio thread.
    //
    // ======================================================

    const bool registered =
        owner.licenseManager.registerInstallation();


    // ======================================================
    // CHECK IF THREAD WAS ASKED TO STOP
    // ======================================================
    //
    // If the plugin is being destroyed while registration
    // has completed, don't start another network operation.
    //
    // ======================================================

    if (threadShouldExit())
    {
        return;
    }


    // ======================================================
    // REGISTRATION FAILED
    // ======================================================

    if (!registered)
    {
        owner.licenseAllowed.store(false);

        return;
    }


    // ======================================================
    // CHECK / CONSUME ONE SESSION
    // ======================================================
    //
    // /use handles:
    //
    //     - activated license
    //     - free-use availability
    //     - incrementing server freeUses
    //
    // Therefore DO NOT call incrementUsage() here.
    //
    // The server has already consumed the use.
    //
    // ======================================================

    const bool allowed =
        owner.licenseManager.checkUsage();


    // ======================================================
    // STORE RESULT FOR AUDIO THREAD
    // ======================================================
    //
    // processBlock() only reads this atomic value.
    //
    // No GUI or network operation occurs on the audio thread.
    //
    // ======================================================

    owner.licenseAllowed.store(allowed);
}


// ==========================================================
// CONSTRUCTOR
// ==========================================================

OfforVocalProAudioProcessor::
OfforVocalProAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput(
                "Input",
                juce::AudioChannelSet::stereo(),
                true
            )
            .withOutput(
                "Output",
                juce::AudioChannelSet::stereo(),
                true
            )
      ),
      apvts(
          *this,
          nullptr,
          "PARAMETERS",
          createParameterLayout()
      )
{
    // ======================================================
    // LICENSE INITIAL STATE
    // ======================================================
    //
    // If a valid activation is already stored locally,
    // allow processing immediately.
    //
    // Otherwise the editor will start the server session
    // check when createEditor() is called.
    //
    // ======================================================

    if (licenseManager.isActivated())
    {
        licenseAllowed.store(true);
    }
}


// ==========================================================
// DESTRUCTOR
// ==========================================================
//
// IMPORTANT:
//
// The license thread must NEVER be allowed to outlive
// this AudioProcessor.
//
// The old detached std::thread could access:
//
//     this
//
// after the processor had already been destroyed.
//
// We now explicitly stop and wait for the JUCE thread.
//
// LicenseManager uses a network timeout, so the shutdown
// may wait while an active HTTP request finishes.
//
// That is intentional: safety is more important than
// allowing the processor to be destroyed underneath a
// running network operation.
//
// ==========================================================

OfforVocalProAudioProcessor::
~OfforVocalProAudioProcessor()
{
    if (licenseSessionThread != nullptr)
    {
        // ==================================================
        // WAIT UNTIL THE LICENSE THREAD HAS COMPLETELY
        // STOPPED BEFORE THIS PROCESSOR IS DESTROYED.
        //
        // -1 = wait indefinitely.
        //
        // LicenseManager has finite network timeouts, so
        // this should not normally block for a long time.
        // ==================================================

        licenseSessionThread->stopThread(-1);

        licenseSessionThread.reset();
    }
}


// ==========================================================
// PARAMETER LAYOUT
// ==========================================================

juce::AudioProcessorValueTreeState::
ParameterLayout
OfforVocalProAudioProcessor::
createParameterLayout()
{
    using Parameter =
        juce::AudioProcessorValueTreeState::ParameterLayout;

    Parameter layout;


    // ======================================================
    // GLOBAL
    // ======================================================

    layout.add(
        std::make_unique<juce::AudioParameterBool>(
            PARAM_BYPASS,
            "Bypass",
            false
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_INPUT,
            "Input",
            juce::NormalisableRange<float>(
                -24.0f,
                24.0f,
                0.01f
            ),
            0.0f,
            "dB"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_OUTPUT,
            "Output",
            juce::NormalisableRange<float>(
                -24.0f,
                24.0f,
                0.01f
            ),
            0.0f,
            "dB"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_MIX,
            "Global Mix",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            100.0f,
            "%"
        )
    );


    // ======================================================
    // TUNER
    // ======================================================

    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_TUNER_KEY,
            "Tuner Key",
            juce::StringArray
            {
                "C",
                "C#",
                "D",
                "D#",
                "E",
                "F",
                "F#",
                "G",
                "G#",
                "A",
                "A#",
                "B"
            },
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_TUNER_SCALE,
            "Tuner Scale",
            juce::StringArray
            {
                "Chromatic",
                "Major",
                "Minor"
            },
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_TUNER_MODE,
            "Tuner Mode",
            juce::StringArray
            {
                "Natural",
                "Modern",
                "Hard Tune"
            },
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_TUNER_RETUNE,
            "Tuner Retune",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            50.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_TUNER_SMOOTH,
            "Tuner Smooth",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            50.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_TUNER_FORMANT,
            "Tuner Formant",
            juce::NormalisableRange<float>(
                -100.0f,
                100.0f,
                1.0f
            ),
            0.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_TUNER_MIX,
            "Tuner Mix",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            100.0f,
            "%"
        )
    );


    // ======================================================
    // DOUBLER
    // ======================================================

    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_DOUBLER_AMOUNT,
            "Doubler Amount",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            0.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_DOUBLER_DETUNE,
            "Doubler Detune",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            50.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_DOUBLER_TIMING,
            "Doubler Timing",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            50.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_DOUBLER_WIDTH,
            "Doubler Width",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            75.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_DOUBLER_MIX,
            "Doubler Mix",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            0.0f,
            "%"
        )
    );


    // ======================================================
    // HARMONY
    // ======================================================

    const juce::StringArray harmonyChoices =
    {
        "Off",
        "3rd Up",
        "5th Up",
        "Octave Up",
        "3rd Down",
        "5th Down",
        "Octave Down"
    };


    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_HARMONY_VOICE1,
            "Harmony Voice 1",
            harmonyChoices,
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_HARMONY_VOICE2,
            "Harmony Voice 2",
            harmonyChoices,
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_HARMONY_VOICE3,
            "Harmony Voice 3",
            harmonyChoices,
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_HARMONY_VOICE4,
            "Harmony Voice 4",
            harmonyChoices,
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_HARMONY_MIX,
            "Harmony Mix",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            0.0f,
            "%"
        )
    );


    // ======================================================
    // CREATIVE FX
    // ======================================================

    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_FX_TYPE,
            "Creative FX Type",
            juce::StringArray
            {
                "Off",
                "Telephone",
                "Radio",
                "Megaphone",
                "Lo-Fi",
                "Whisper",
                "Robot",
                "Dark",
                "Bright",
                "Distorted",
                "Wide",
                "Dream"
            },
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_FX_AMOUNT,
            "Creative FX Amount",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            0.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_FX_MIX,
            "Creative FX Mix",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            0.0f,
            "%"
        )
    );


    // ======================================================
    // SPACE
    // ======================================================

    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_SPACE_TYPE,
            "Space Type",
            juce::StringArray
            {
                "Off",
                "Room",
                "Plate",
                "Hall",
                "Dark",
                "Air",
                "Dream"
            },
            0
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_SPACE_SIZE,
            "Space Size",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            50.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_SPACE_DECAY,
            "Space Decay",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            50.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_SPACE_PREDELAY,
            "Space Pre-delay",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            20.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_SPACE_DAMPING,
            "Space Damping",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            50.0f,
            "%"
        )
    );


    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_SPACE_MIX,
            "Space Mix",
            juce::NormalisableRange<float>(
                0.0f,
                100.0f,
                0.01f
            ),
            0.0f,
            "%"
        )
    );


    return layout;
}


// ==========================================================
// PREPARE TO PLAY
// ==========================================================

void OfforVocalProAudioProcessor::prepareToPlay(
    double newSampleRate,
    int samplesPerBlock)
{
    // ==========================================================
    // HOST INFORMATION
    // ==========================================================

    currentSampleRate =
        newSampleRate > 0.0
            ? newSampleRate
            : 44100.0;

    currentBlockSize =
        juce::jmax(1, samplesPerBlock);


    // ==========================================================
    // PITCH DETECTOR
    // ==========================================================
    //
    // IMPORTANT:
    //
    // Pitch detection remains at the HOST sample rate.
    //
    // We do NOT oversample the tuner because the existing
    // pitch-detection system is designed around the host rate.
    //
    // ==========================================================

    pitchDetector.prepare(
        currentSampleRate,
        currentBlockSize);


    // ==========================================================
    // PITCH CORRECTOR
    // ==========================================================

    pitchCorrector.prepare(
        currentSampleRate,
        currentBlockSize,
        2);


    // ==========================================================
    // DRY BUFFER
    // ==========================================================

    dryBuffer.setSize(
        getTotalNumInputChannels(),
        currentBlockSize);

    dryBuffer.clear();


    // ==========================================================
    // RESET TUNER STATE
    // ==========================================================

    targetMidiNote.store(60.0);
    targetPitchClass.store(0);

    previousTargetMidiNote = 60.0;
    smoothedTargetMidiNote = 60.0;


    // ==========================================================
    // RESET LEVEL METERS
    // ==========================================================

    inputLevelDb.store(-60.0f);
    outputLevelDb.store(-60.0f);


    // ==========================================================
    // OVERSAMPLING ENGINE
    // ==========================================================
    //
    // configureOversampling() prepares the creative DSP chain
    // at the correct internal sample rate.
    //
    // Example at a 48 kHz host:
    //
    //     Off -> 48 kHz
    //     2X  -> 96 kHz
    //     4X  -> 192 kHz
    //     8X  -> 384 kHz
    //
    // ==========================================================

    const juce::SpinLock::ScopedLockType lock(
        dspConfigurationLock);

    configureOversampling(
        oversamplingMode);

    // ==========================================================
    // APPLY CURRENT PROCESSING QUALITY
    // ==========================================================

    DoublerProcessor::ProcessingQuality dspQuality =
        DoublerProcessor::ProcessingQuality::High;

    if (processingQuality.equalsIgnoreCase("Low"))
    {
        dspQuality =
            DoublerProcessor::ProcessingQuality::Low;
    }
    else if (processingQuality.equalsIgnoreCase("Medium"))
    {
        dspQuality =
            DoublerProcessor::ProcessingQuality::Medium;
    }
    else if (processingQuality.equalsIgnoreCase("High"))
    {
        dspQuality =
            DoublerProcessor::ProcessingQuality::High;
    }
    else if (processingQuality.equalsIgnoreCase("Ultra"))
    {
        dspQuality =
            DoublerProcessor::ProcessingQuality::Ultra;
    }

    doublerProcessor.setProcessingQuality(
        dspQuality
    );


    // ==========================================================
    // APPLY CURRENT CPU MODE
    // ==========================================================

    DoublerProcessor::CPUMode dspCPUMode =
        DoublerProcessor::CPUMode::Balanced;

    if (cpuMode.equalsIgnoreCase("Low CPU"))
    {
        dspCPUMode =
            DoublerProcessor::CPUMode::LowCPU;
    }
    else if (cpuMode.equalsIgnoreCase("Balanced"))
    {
        dspCPUMode =
            DoublerProcessor::CPUMode::Balanced;
    }
    else if (cpuMode.equalsIgnoreCase("Performance"))
    {
        dspCPUMode =
            DoublerProcessor::CPUMode::Performance;
    }

    doublerProcessor.setCPUMode(
        dspCPUMode
    );


    // ==========================================================
    // APPLY CURRENT PROCESSING QUALITY TO HARMONY
    // ==========================================================

    HarmonyProcessor::ProcessingQuality harmonyQuality =
        HarmonyProcessor::ProcessingQuality::High;


    if (processingQuality.equalsIgnoreCase("Low"))
    {
        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::Low;
    }
    else if (processingQuality.equalsIgnoreCase("Medium"))
    {
        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::Medium;
    }
    else if (processingQuality.equalsIgnoreCase("High"))
    {
        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::High;
    }
    else if (processingQuality.equalsIgnoreCase("Ultra"))
    {
        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::Ultra;
    }


    harmonyProcessor.setProcessingQuality(
        harmonyQuality
    );


    // ==========================================================
    // APPLY CURRENT CPU MODE TO HARMONY
    // ==========================================================

    HarmonyProcessor::CPUMode harmonyMode =
        HarmonyProcessor::CPUMode::Balanced;


    if (cpuMode.equalsIgnoreCase("Low CPU"))
    {
        harmonyMode =
            HarmonyProcessor::CPUMode::LowCPU;
    }
    else if (cpuMode.equalsIgnoreCase("Balanced"))
    {
        harmonyMode =
            HarmonyProcessor::CPUMode::Balanced;
    }
    else if (cpuMode.equalsIgnoreCase("Performance"))
    {
        harmonyMode =
            HarmonyProcessor::CPUMode::Performance;
    }


    harmonyProcessor.setCPUMode(
        harmonyMode
    );


    // ==========================================================
    // APPLY CURRENT PROCESSING QUALITY TO CREATIVE FX
    // ==========================================================

    CreativeFXProcessor::ProcessingQuality creativeFXQuality =
        CreativeFXProcessor::ProcessingQuality::High;


    if (processingQuality.equalsIgnoreCase("Low"))
    {
        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::Low;
    }
    else if (processingQuality.equalsIgnoreCase("Medium"))
    {
        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::Medium;
    }
    else if (processingQuality.equalsIgnoreCase("High"))
    {
        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::High;
    }
    else if (processingQuality.equalsIgnoreCase("Ultra"))
    {
        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::Ultra;
    }


    creativeFXProcessor.setProcessingQuality(
        creativeFXQuality
    );


    // ==========================================================
    // APPLY CURRENT CPU MODE TO CREATIVE FX
    // ==========================================================

    CreativeFXProcessor::CPUMode creativeFXMode =
        CreativeFXProcessor::CPUMode::Balanced;


    if (cpuMode.equalsIgnoreCase("Low CPU"))
    {
        creativeFXMode =
            CreativeFXProcessor::CPUMode::LowCPU;
    }
    else if (cpuMode.equalsIgnoreCase("Balanced"))
    {
        creativeFXMode =
            CreativeFXProcessor::CPUMode::Balanced;
    }
    else if (cpuMode.equalsIgnoreCase("Performance"))
    {
        creativeFXMode =
            CreativeFXProcessor::CPUMode::Performance;
    }


    creativeFXProcessor.setCPUMode(
        creativeFXMode
    );
}


// ==========================================================
// RELEASE RESOURCES
// ==========================================================

void OfforVocalProAudioProcessor::releaseResources()
{
    // ==========================================================
    // PROTECT DSP RECONFIGURATION
    // ==========================================================

    const juce::SpinLock::ScopedLockType lock(
        dspConfigurationLock);


    // ==========================================================
    // RESET HOST-RATE DSP
    // ==========================================================

    pitchDetector.reset();
    pitchCorrector.reset();


    // ==========================================================
    // RESET CREATIVE DSP
    // ==========================================================

    doublerProcessor.reset();
    harmonyProcessor.reset();
    creativeFXProcessor.reset();
    spaceProcessor.reset();


    // ==========================================================
    // RESET OVERSAMPLER
    // ==========================================================

    if (oversampler != nullptr)
        oversampler->reset();

    oversampler.reset();


    // ==========================================================
    // CLEAR DRY BUFFER
    // ==========================================================

    dryBuffer.setSize(0, 0);


    // ==========================================================
    // REMOVE REPORTED LATENCY
    // ==========================================================

    setLatencySamples(0);
}

// ==========================================================
// GET OVERSAMPLING FACTOR
// ==========================================================
//
// Returns the actual sample-rate multiplier:
//
//     Off -> 1
//     2X  -> 2
//     4X  -> 4
//     8X  -> 8
//
// ==========================================================

int OfforVocalProAudioProcessor::getOversamplingFactorFromMode(
    const juce::String& mode) const
{
    if (mode == "2X")
        return 2;

    if (mode == "4X")
        return 4;

    if (mode == "8X")
        return 8;

    return 1;
}


// ==========================================================
// CONFIGURE OVERSAMPLING ENGINE
// ==========================================================

void OfforVocalProAudioProcessor::configureOversampling(
    const juce::String& mode)
{
    // ==========================================================
    // DETERMINE FACTOR
    // ==========================================================

    const int factor =
        getOversamplingFactorFromMode(mode);


    // ==========================================================
    // OFF
    // ==========================================================
    //
    // No oversampling.
    //
    // Creative DSP runs at the host sample rate.
    //
    // ==========================================================

    if (factor == 1)
    {
        oversampler.reset();


        // ------------------------------------------------------
        // DOUBLER
        // ------------------------------------------------------

        doublerProcessor.prepare(
            currentSampleRate,
            currentBlockSize,
            2);


        // ------------------------------------------------------
        // HARMONY
        // ------------------------------------------------------

        harmonyProcessor.prepare(
            currentSampleRate,
            currentBlockSize,
            2);


        // ------------------------------------------------------
        // CREATIVE FX
        // ------------------------------------------------------

        creativeFXProcessor.prepare(
            currentSampleRate,
            currentBlockSize,
            2);


        // ------------------------------------------------------
        // SPACE
        // ------------------------------------------------------

        spaceProcessor.prepare(
            currentSampleRate,
            currentBlockSize,
            2);


        // ------------------------------------------------------
        // NO EXTRA LATENCY
        // ------------------------------------------------------

        setLatencySamples(0);

        return;
    }


    // ==========================================================
    // JUCE OVERSAMPLING STAGES
    // ==========================================================
    //
    // JUCE's constructor uses:
    //
    //     1 stage = 2X
    //     2 stages = 4X
    //     3 stages = 8X
    //
    // ==========================================================

    const int stages =
        factor == 2
            ? 1
            : factor == 4
                ? 2
                : 3;


    // ==========================================================
    // CREATE OVERSAMPLER
    // ==========================================================
    //
    // Polyphase IIR gives us lower latency than the FIR option.
    //
    // useIntegerLatency = true
    //
    // This makes the reported latency an integer number of
    // host samples, which is useful for plugin host latency
    // compensation.
    //
    // ==========================================================

    oversampler =
        std::make_unique<
            juce::dsp::Oversampling<float>>(
                2,
                stages,
                juce::dsp::Oversampling<float>::
                    filterHalfBandPolyphaseIIR,
                true,
                true);


    // ==========================================================
    // PREPARE OVERSAMPLER
    // ==========================================================

    oversampler->initProcessing(
        static_cast<size_t>(
            currentBlockSize));

    oversampler->reset();


    // ==========================================================
    // INTERNAL SAMPLE RATE
    // ==========================================================

    const double internalSampleRate =
        currentSampleRate *
        static_cast<double>(factor);


    // ==========================================================
    // INTERNAL BLOCK SIZE
    // ==========================================================

    const int internalBlockSize =
        currentBlockSize * factor;


    // ==========================================================
    // PREPARE DOUBLER AT INTERNAL RATE
    // ==========================================================

    doublerProcessor.prepare(
        internalSampleRate,
        internalBlockSize,
        2);


    // ==========================================================
    // PREPARE HARMONY AT INTERNAL RATE
    // ==========================================================

    harmonyProcessor.prepare(
        internalSampleRate,
        internalBlockSize,
        2);


    // ==========================================================
    // PREPARE CREATIVE FX AT INTERNAL RATE
    // ==========================================================

    creativeFXProcessor.prepare(
        internalSampleRate,
        internalBlockSize,
        2);


    // ==========================================================
    // PREPARE SPACE AT INTERNAL RATE
    // ==========================================================

    spaceProcessor.prepare(
        internalSampleRate,
        internalBlockSize,
        2);


    // ==========================================================
    // REPORT OVERSAMPLING LATENCY TO THE DAW
    // ==========================================================

    const double latency =
        oversampler->getLatencyInSamples();

    setLatencySamples(
        juce::jmax(
            0,
            static_cast<int>(
                std::ceil(latency))));


    // ==========================================================
    // DEBUG INFORMATION
    // ==========================================================

   #if JUCE_DEBUG

    DBG(
        "==========================================");

    DBG(
        "OFFOR VOCAL PRO OVERSAMPLING");

    DBG(
        "Mode: "
        + mode);

    DBG(
        "Host sample rate: "
        + juce::String(currentSampleRate));

    DBG(
        "Internal sample rate: "
        + juce::String(internalSampleRate));

    DBG(
        "Host block size: "
        + juce::String(currentBlockSize));

    DBG(
        "Internal block size: "
        + juce::String(internalBlockSize));

    DBG(
        "Oversampling factor: "
        + juce::String(factor));

    DBG(
        "Latency: "
        + juce::String(latency));

    DBG(
        "==========================================");

   #endif
}

// ==========================================================
// BUS LAYOUT
// ==========================================================

bool
OfforVocalProAudioProcessor::
isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto& mainInput =
        layouts.getChannelSet(
            true,
            0
        );


    const auto& mainOutput =
        layouts.getChannelSet(
            false,
            0
        );


    if (mainInput !=
        juce::AudioChannelSet::stereo())
    {
        return false;
    }


    if (mainOutput !=
        juce::AudioChannelSet::stereo())
    {
        return false;
    }


    return true;
}


// ==========================================================
// MIDI → FREQUENCY
// ==========================================================

double
OfforVocalProAudioProcessor::
midiToFrequency(
    double midiNote)
{
    return 440.0 *
           std::pow(
               2.0,
               (midiNote - 69.0) / 12.0
           );
}


// ==========================================================
// SCALE MEMBERSHIP
// ==========================================================

bool
OfforVocalProAudioProcessor::
isPitchClassInScale(
    int pitchClass,
    int key,
    ScaleType scale) const
{
    pitchClass =
        ((pitchClass % 12) + 12) % 12;


    key =
        ((key % 12) + 12) % 12;


    // ======================================================
    // CHROMATIC
    // ======================================================

    if (scale == ScaleType::Chromatic)
        return true;


    const int relativePitch =
        (pitchClass - key + 12) % 12;


    // ======================================================
    // MAJOR
    //
    // 0  2  4  5  7  9  11
    // ======================================================

    if (scale == ScaleType::Major)
    {
        switch (relativePitch)
        {
            case 0:
            case 2:
            case 4:
            case 5:
            case 7:
            case 9:
            case 11:
                return true;

            default:
                return false;
        }
    }


    // ======================================================
    // NATURAL MINOR
    //
    // 0  2  3  5  7  8  10
    // ======================================================

    if (scale == ScaleType::Minor)
    {
        switch (relativePitch)
        {
            case 0:
            case 2:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
                return true;

            default:
                return false;
        }
    }


    return true;
}


// ==========================================================
// FIND NEAREST SCALE PITCH CLASS
// ==========================================================

int
OfforVocalProAudioProcessor::
findNearestScalePitchClass(
    int detectedPitchClass,
    int key,
    ScaleType scale) const
{
    detectedPitchClass =
        ((detectedPitchClass % 12) + 12) % 12;


    key =
        ((key % 12) + 12) % 12;


    if (scale == ScaleType::Chromatic)
        return detectedPitchClass;


    int bestPitchClass =
        detectedPitchClass;

    int bestDistance =
        13;


    for (int pitchClass = 0;
         pitchClass < 12;
         ++pitchClass)
    {
        if (!isPitchClassInScale(
                pitchClass,
                key,
                scale))
        {
            continue;
        }


        int distance =
            std::abs(
                pitchClass -
                detectedPitchClass
            );


        // Circular distance around octave.
        distance =
            std::min(
                distance,
                12 - distance
            );


        if (distance < bestDistance)
        {
            bestDistance = distance;

            bestPitchClass =
                pitchClass;
        }
    }


    return bestPitchClass;
}


// ==========================================================
// TARGET NOTE SELECTOR
// ==========================================================

double
OfforVocalProAudioProcessor::
selectTargetMidiNote(
    double detectedMidiNote,
    int key,
    ScaleType scale) const
{
    if (!std::isfinite(detectedMidiNote))
        return previousTargetMidiNote;


    // ======================================================
    // LIMIT DETECTED RANGE
    // ======================================================

    detectedMidiNote =
        juce::jlimit(
            24.0,
            108.0,
            detectedMidiNote
        );


    // ======================================================
    // CHROMATIC
    // ======================================================

    if (scale == ScaleType::Chromatic)
    {
        return juce::jlimit(
            24.0,
            108.0,
            std::round(detectedMidiNote)
        );
    }


    // ======================================================
    // FIND NEAREST SCALE NOTE
    // ======================================================

    const int baseMidi =
        juce::roundToInt(
            detectedMidiNote
        );


    double bestTarget =
        static_cast<double>(baseMidi);

    double bestDistance =
        std::numeric_limits<double>::max();


    // Search a small MIDI range around the detected note.
    //
    // This guarantees that we choose the closest legal
    // note in the selected scale.

    for (int candidate = baseMidi - 12;
         candidate <= baseMidi + 12;
         ++candidate)
    {
        if (candidate < 24 ||
            candidate > 108)
        {
            continue;
        }


        const int candidatePitchClass =
            ((candidate % 12) + 12) % 12;


        if (!isPitchClassInScale(
                candidatePitchClass,
                key,
                scale))
        {
            continue;
        }


        const double distance =
            std::abs(
                detectedMidiNote -
                static_cast<double>(candidate)
            );


        if (distance < bestDistance)
        {
            bestDistance = distance;

            bestTarget =
                static_cast<double>(candidate);
        }
    }


    return juce::jlimit(
        24.0,
        108.0,
        bestTarget
    );
}

// ==========================================================
// PROCESS BLOCK
// ==========================================================

void
OfforVocalProAudioProcessor::
processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(
        midiMessages
    );


    // ======================================================
    // LICENSE GATE
    // ======================================================
    //
    // IMPORTANT:
    //
    // NO network calls happen here.
    //
    // The audio thread only reads an atomic boolean that
    // was updated by the background license thread.
    //
    // While the server check is running, processing is held.
    //
    // Once the server approves the session, DSP starts.
    //
    // If the 10 free uses are exhausted, processing remains
    // disabled until a valid license is activated.
    //
    // ======================================================

    if (!licenseAllowed.load())
    {
        buffer.clear();

        return;
    }


    const int numSamples =
        buffer.getNumSamples();


    const int numChannels =
        buffer.getNumChannels();


    if (numSamples <= 0 ||
        numChannels <= 0)
    {
        return;
    }


    // ==========================================================
    // SETTINGS - ENABLE PROCESSING
    // ==========================================================
    //
    // This is separate from the main APVTS BYPASS parameter.
    //
    // BYPASS:
    //     Plugin's normal audio bypass control.
    //
    // ENABLE PROCESSING:
    //     User preference from Settings.
    //
    // When processing is disabled, leave the incoming audio
    // untouched.
    //

    if (!processingEnabled.load())
        return;


    // ======================================================
    // BYPASS
    // ======================================================

    const auto* bypassParameter =
        apvts.getRawParameterValue(
            PARAM_BYPASS
        );


    if (bypassParameter != nullptr &&
        bypassParameter->load() > 0.5f)
    {
        return;
    }


    // ======================================================
    // READ GLOBAL PARAMETERS
    // ======================================================

    const float inputDb =
        apvts.getRawParameterValue(
            PARAM_INPUT
        )->load();


    const float outputDb =
        apvts.getRawParameterValue(
            PARAM_OUTPUT
        )->load();


    const float globalMix =
        apvts.getRawParameterValue(
            PARAM_MIX
        )->load() / 100.0f;


    // ======================================================
    // READ TUNER PARAMETERS
    // ======================================================

    const int key =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_TUNER_KEY
            )->load()
        );


    const int scaleIndex =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_TUNER_SCALE
            )->load()
        );


    const int modeIndex =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_TUNER_MODE
            )->load()
        );


    const float retune =
        apvts.getRawParameterValue(
            PARAM_TUNER_RETUNE
        )->load();


    const float smooth =
        apvts.getRawParameterValue(
            PARAM_TUNER_SMOOTH
        )->load();


    const float tunerMix =
        apvts.getRawParameterValue(
            PARAM_TUNER_MIX
        )->load() / 100.0f;


    const ScaleType scale =
        static_cast<ScaleType>(
            juce::jlimit(
                0,
                2,
                scaleIndex
            )
        );


    // ======================================================
    // TUNER MODE
    // ======================================================

    const TunerMode mode =
        static_cast<TunerMode>(
            juce::jlimit(
                0,
                2,
                modeIndex
            )
        );


    // ======================================================
    // MODE-DEPENDENT CORRECTION STRENGTH
    //
    // Natural   = gentle correction
    // Modern    = strong but musical correction
    // Hard Tune = full correction
    // ======================================================

    float correctionStrength = 1.0f;


    switch (mode)
    {
        case TunerMode::Natural:
            correctionStrength = 0.55f;
            break;


        case TunerMode::Modern:
            correctionStrength = 0.80f;
            break;


        case TunerMode::HardTune:
            correctionStrength = 1.0f;
            break;
    }


    // ==========================================================
    // INPUT LEVEL METER
    // ==========================================================
    //
    // Measure the incoming signal BEFORE the plugin's INPUT
    // gain so the meter represents the actual signal entering
    // Offor Vocal Pro.
    //
    // ==========================================================

    float inputPeak = 0.0f;


    for (int channel = 0;
         channel < buffer.getNumChannels();
         ++channel)
    {
        inputPeak =
            juce::jmax(
                inputPeak,
                buffer.getMagnitude(
                    channel,
                    0,
                    buffer.getNumSamples()
                )
            );
    }


    const float inputMeterDb =
        inputPeak > 0.000001f
            ? juce::Decibels::gainToDecibels(
                inputPeak
              )
            : -60.0f;


    inputLevelDb.store(
        juce::jlimit(
            -60.0f,
            0.0f,
            inputMeterDb
        )
    );


    // ======================================================
    // INPUT GAIN
    //
    // Input gain is deliberately applied BEFORE pitch
    // detection so the detector receives the actual
    // signal level being processed.
    // ======================================================

    const float inputGain =
        juce::Decibels::decibelsToGain(
            inputDb
        );


    buffer.applyGain(
        inputGain
    );


    // ======================================================
    // STORE DRY SIGNAL
    //
    // This is the dry signal after input gain but before
    // tuner processing.
    // ======================================================

    if (dryBuffer.getNumChannels() != numChannels ||
        dryBuffer.getNumSamples() != numSamples)
    {
        dryBuffer.setSize(
            numChannels,
            numSamples,
            false,
            false,
            true
        );
    }


    dryBuffer.makeCopyOf(
        buffer,
        true
    );


    // ======================================================
    // PITCH DETECTION
    // ======================================================

    pitchDetector.processBlock(
        buffer
    );


    const bool pitchDetected =
        pitchDetector.isPitchDetected();


    const double detectedMidi =
        pitchDetector.getMidiNote();


    // ======================================================
    // TARGET NOTE SELECTION
    // ======================================================

    double selectedTargetMidi =
        smoothedTargetMidiNote;


    if (pitchDetected &&
        std::isfinite(detectedMidi))
    {
        // --------------------------------------------------
        // Find the musical target note first.
        // --------------------------------------------------

        const double rawTargetMidi =
            selectTargetMidiNote(
                detectedMidi,
                key,
                scale
            );


        // --------------------------------------------------
        // Smooth the TARGET NOTE.
        //
        // 0%   = fast target movement
        // 50%  = balanced
        // 100% = very stable
        // --------------------------------------------------

        const float smoothAmount =
            juce::jlimit(
                0.0f,
                1.0f,
                smooth / 100.0f
            );


        const double smoothingCoefficient =
            0.60
            * (1.0 -
               static_cast<double>(
                   smoothAmount
               ))
            +
            0.08
            * static_cast<double>(
                smoothAmount
              );


        // --------------------------------------------------
        // Detect a genuine note change.
        // --------------------------------------------------

        const double noteDistance =
            std::abs(
                rawTargetMidi -
                smoothedTargetMidiNote
            );


        if (noteDistance >= 11.0)
        {
            // Large jump: move immediately to the new
            // musical region instead of slowly travelling
            // through an octave.

            smoothedTargetMidiNote =
                rawTargetMidi;
        }
        else
        {
            smoothedTargetMidiNote +=
                (
                    rawTargetMidi -
                    smoothedTargetMidiNote
                )
                *
                smoothingCoefficient;
        }


        // --------------------------------------------------
        // Keep the target in the valid vocal MIDI range.
        // --------------------------------------------------

        smoothedTargetMidiNote =
            juce::jlimit(
                24.0,
                108.0,
                smoothedTargetMidiNote
            );


        selectedTargetMidi =
            smoothedTargetMidiNote;


        previousTargetMidiNote =
            selectedTargetMidi;


        targetMidiNote.store(
            selectedTargetMidi
        );


        targetPitchClass.store(
            (
                juce::roundToInt(
                    selectedTargetMidi
                )
                % 12
                + 12
            )
            % 12
        );
    }


    // ======================================================
    // TUNER PROCESSING
    // ======================================================

    if (tunerMix > 0.0f)
    {
        const double detectedFrequency =
            pitchDetector.getFrequencyHz();


        const float safeFrequency =
            (
                detectedFrequency > 0.0 &&
                std::isfinite(detectedFrequency)
            )
                ? static_cast<float>(
                    detectedFrequency
                  )
                : 0.0f;


        // Always run the corrector while the tuner is enabled.
        //
        // PitchCorrector handles the transition between
        // corrected and uncorrected audio smoothly.

        pitchCorrector.processBlock(
            buffer,
            safeFrequency,
            static_cast<float>(
                selectedTargetMidi
            ),
            correctionStrength,
            retune,
            pitchDetected
        );


        // ==================================================
        // TUNER MIX
        // ==================================================

        const float processedGain =
            tunerMix;


        const float dryGain =
            1.0f - tunerMix;


        for (int channel = 0;
             channel < numChannels;
             ++channel)
        {
            auto* processed =
                buffer.getWritePointer(
                    channel
                );


            const auto* dry =
                dryBuffer.getReadPointer(
                    channel
                );


            for (int sample = 0;
                 sample < numSamples;
                 ++sample)
            {
                processed[sample] =
                    dry[sample] * dryGain
                    +
                    processed[sample] *
                        processedGain;
            }
        }
    }
    else
    {
        buffer.makeCopyOf(
            dryBuffer,
            true
        );
    }


    
    // ==========================================================
    // CREATIVE DSP - OVERSAMPLED ENGINE
    // ==========================================================
    //
    // IMPORTANT:
    //
    // Pitch detection and pitch correction have already happened
    // at the HOST sample rate above.
    //
    // From this point onward:
    //
    //     Doubler
    //     Harmony
    //     Creative FX
    //     Space
    //
    // run through the real oversampling engine when the user has
    // selected 2X, 4X or 8X.
    //
    // If Oversampling = Off:
    //
    //     All four processors run normally at the host rate.
    //
    // IMPORTANT:
    // This is ONE combined DSP section.
    // Do NOT keep separate Doubler/Harmony/FX/Space processing
    // sections below this block.
    //
    // ==========================================================


    // ==========================================================
    // READ DOUBLER PARAMETERS
    // ==========================================================

    const float doublerAmount =
        apvts.getRawParameterValue(
            PARAM_DOUBLER_AMOUNT
        )->load();

    const float doublerDetune =
        apvts.getRawParameterValue(
            PARAM_DOUBLER_DETUNE
        )->load();

    const float doublerTiming =
        apvts.getRawParameterValue(
            PARAM_DOUBLER_TIMING
        )->load();

    const float doublerWidth =
        apvts.getRawParameterValue(
            PARAM_DOUBLER_WIDTH
        )->load();

    const float doublerMix =
        apvts.getRawParameterValue(
            PARAM_DOUBLER_MIX
        )->load();


    // ==========================================================
    // READ HARMONY PARAMETERS
    // ==========================================================

    const int harmonyVoice1Choice =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_HARMONY_VOICE1
            )->load()
        );

    const int harmonyVoice2Choice =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_HARMONY_VOICE2
            )->load()
        );

    const int harmonyVoice3Choice =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_HARMONY_VOICE3
            )->load()
        );

    const int harmonyVoice4Choice =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_HARMONY_VOICE4
            )->load()
        );


    // Harmony Mix is a 0-100 UI parameter.
    // HarmonyProcessor expects 0-1.

    const float harmonyMix =
        apvts.getRawParameterValue(
            PARAM_HARMONY_MIX
        )->load() / 100.0f;


    const float harmonyDetectedFrequency =
        static_cast<float>(
            pitchDetector.getFrequencyHz()
        );


    const bool harmonyPitchDetected =
        pitchDetector.isPitchDetected();


    const double harmonyDetectedMidi =
        pitchDetector.getMidiNote();


    const int harmonyKey =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_TUNER_KEY
            )->load()
        );


    const int harmonyScale =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_TUNER_SCALE
            )->load()
        );


    // ==========================================================
    // SCALE-AWARE HARMONY CALCULATION
    // ==========================================================
    //
    // This is the same harmony calculation you already had.
    //
    // It happens at the HOST rate because it only calculates
    // MIDI intervals. It does not process audio.
    //
    // ==========================================================

    auto calculateScaleAwareHarmonyInterval =
        [harmonyKey, harmonyScale](
            double detectedMidi,
            int choice) -> float
    {
        if (choice == 0 ||
            !std::isfinite(detectedMidi))
        {
            return 0.0f;
        }


        // ======================================================
        // CHROMATIC
        // ======================================================

        if (harmonyScale == 0)
        {
            switch (choice)
            {
                case 1:
                    return 3.0f;

                case 2:
                    return 7.0f;

                case 3:
                    return 12.0f;

                case 4:
                    return -3.0f;

                case 5:
                    return -7.0f;

                case 6:
                    return -12.0f;

                default:
                    return 0.0f;
            }
        }


        // ======================================================
        // SCALE DEFINITION
        // ======================================================

        static constexpr int majorScale[7] =
        {
            0,
            2,
            4,
            5,
            7,
            9,
            11
        };


        static constexpr int minorScale[7] =
        {
            0,
            2,
            3,
            5,
            7,
            8,
            10
        };


        const int* scaleIntervals =
            (harmonyScale == 2)
                ? minorScale
                : majorScale;


        // ======================================================
        // FIND NEAREST ACTUAL SCALE NOTE
        // ======================================================

        const int roundedMidi =
            juce::jlimit(
                0,
                127,
                juce::roundToInt(
                    static_cast<float>(
                        detectedMidi
                    )
                )
            );


        int nearestMidi =
            roundedMidi;


        int smallestDistance =
            1000;


        for (int octave = -1;
            octave <= 10;
            ++octave)
        {
            for (int degree = 0;
                degree < 7;
                ++degree)
            {
                const int candidateMidi =
                    harmonyKey
                    +
                    octave * 12
                    +
                    scaleIntervals[degree];


                if (candidateMidi < 0 ||
                    candidateMidi > 127)
                {
                    continue;
                }


                const int distance =
                    std::abs(
                        candidateMidi -
                        roundedMidi
                    );


                if (distance <
                    smallestDistance)
                {
                    smallestDistance =
                        distance;

                    nearestMidi =
                        candidateMidi;
                }
            }
        }


        // ======================================================
        // FIND SOURCE SCALE DEGREE
        // ======================================================

        const int sourcePitchClass =
            ((nearestMidi % 12) + 12) % 12;


        int sourceDegree = 0;


        for (int degree = 0;
            degree < 7;
            ++degree)
        {
            const int scalePitchClass =
                (
                    (
                        harmonyKey +
                        scaleIntervals[degree]
                    )
                    % 12
                    + 12
                )
                % 12;


            if (scalePitchClass ==
                sourcePitchClass)
            {
                sourceDegree =
                    degree;

                break;
            }
        }


        // ======================================================
        // DETERMINE SCALE STEP DISTANCE
        // ======================================================

        int scaleSteps = 0;


        switch (choice)
        {
            case 1:
                scaleSteps = 2;
                break;

            case 2:
                scaleSteps = 4;
                break;

            case 3:
                scaleSteps = 7;
                break;

            case 4:
                scaleSteps = -2;
                break;

            case 5:
                scaleSteps = -4;
                break;

            case 6:
                scaleSteps = -7;
                break;

            default:
                return 0.0f;
        }


        // ======================================================
        // DESTINATION SCALE DEGREE
        // ======================================================

        const int targetAbsoluteDegree =
            sourceDegree +
            scaleSteps;


        const int octaveOffset =
            static_cast<int>(
                std::floor(
                    static_cast<double>(
                        targetAbsoluteDegree
                    ) / 7.0
                )
            );


        int targetDegree =
            targetAbsoluteDegree -
            octaveOffset * 7;


        if (targetDegree < 0)
            targetDegree += 7;


        // ======================================================
        // BUILD TARGET MIDI NOTE
        // ======================================================

        const int sourceOctave =
            static_cast<int>(
                std::floor(
                    static_cast<double>(
                        nearestMidi
                    ) / 12.0
                )
            );


        const int targetMidi =
            harmonyKey
            +
            (sourceOctave + octaveOffset) * 12
            +
            scaleIntervals[targetDegree];


        // ======================================================
        // RETURN ACTUAL INTERVAL
        // ======================================================

        return static_cast<float>(
            static_cast<double>(
                targetMidi
            )
            -
            detectedMidi
        );
    };


    // ==========================================================
    // CALCULATE ALL FOUR HARMONY VOICES
    // ==========================================================

    const float harmonyVoice1 =
        calculateScaleAwareHarmonyInterval(
            harmonyDetectedMidi,
            harmonyVoice1Choice
        );


    const float harmonyVoice2 =
        calculateScaleAwareHarmonyInterval(
            harmonyDetectedMidi,
            harmonyVoice2Choice
        );


    const float harmonyVoice3 =
        calculateScaleAwareHarmonyInterval(
            harmonyDetectedMidi,
            harmonyVoice3Choice
        );


    const float harmonyVoice4 =
        calculateScaleAwareHarmonyInterval(
            harmonyDetectedMidi,
            harmonyVoice4Choice
        );


    // ==========================================================
    // CHECK WHETHER HARMONY HAS AN ACTIVE VOICE
    // ==========================================================

    const float harmonyIntervals[4] =
    {
        harmonyVoice1,
        harmonyVoice2,
        harmonyVoice3,
        harmonyVoice4
    };


    bool hasHarmonyVoice = false;


    for (const auto interval :
        harmonyIntervals)
    {
        if (std::abs(interval) > 0.01f)
        {
            hasHarmonyVoice = true;
            break;
        }
    }


    // ==========================================================
    // READ CREATIVE FX PARAMETERS
    // ==========================================================

    const int creativeFXTypeValue =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_FX_TYPE
            )->load()
        );


    const float creativeFXAmount =
        apvts.getRawParameterValue(
            PARAM_FX_AMOUNT
        )->load() / 100.0f;


    const float creativeFXMix =
        apvts.getRawParameterValue(
            PARAM_FX_MIX
        )->load() / 100.0f;


    // ----------------------------------------------------------
    // Convert APVTS integer parameter into the strongly typed
    // CreativeFXProcessor::Type enum.
    // ----------------------------------------------------------

    const auto creativeFXType =
        static_cast<
            CreativeFXProcessor::Type
        >(
            juce::jlimit(
                0,
                static_cast<int>(
                    CreativeFXProcessor::Type::Dream
                ),
                creativeFXTypeValue
            )
        );


    // ==========================================================
    // READ SPACE PARAMETERS
    // ==========================================================

    const int spaceType =
        juce::roundToInt(
            apvts.getRawParameterValue(
                PARAM_SPACE_TYPE
            )->load()
        );


    const float spaceSize =
        apvts.getRawParameterValue(
            PARAM_SPACE_SIZE
        )->load() / 100.0f;


    const float spaceDecay =
        apvts.getRawParameterValue(
            PARAM_SPACE_DECAY
        )->load() / 100.0f;


    const float spacePreDelay =
        apvts.getRawParameterValue(
            PARAM_SPACE_PREDELAY
        )->load() / 100.0f;


    const float spaceDamping =
        apvts.getRawParameterValue(
            PARAM_SPACE_DAMPING
        )->load() / 100.0f;


    const float spaceMix =
        apvts.getRawParameterValue(
            PARAM_SPACE_MIX
        )->load() / 100.0f;


    // ==========================================================
    // OVERSAMPLED PROCESSING
    // ==========================================================
    //
    // If an oversampler exists:
    //
    //     HOST RATE
    //          |
    //          v
    //       UPSAMPLE
    //          |
    //          v
    //     CREATIVE DSP
    //          |
    //          v
    //      DOWNSAMPLE
    //          |
    //          v
    //       HOST RATE
    //
    // ==========================================================

    if (oversampler != nullptr)
    {
        // ======================================================
        // CREATE HOST AUDIO BLOCK
        // ======================================================

        juce::dsp::AudioBlock<float> hostBlock(
            buffer
        );


        // ======================================================
        // UPSAMPLE
        // ======================================================

        auto oversampledBlock =
            oversampler->processSamplesUp(
                hostBlock
            );


        // ======================================================
        // CREATE AUDIOBUFFER VIEW
        // ======================================================
        //
        // The existing DSP processors use AudioBuffer<float>.
        //
        // We create a lightweight view into the oversampler's
        // internal memory.
        //
        // No additional audio allocation is performed here.
        //
        // ======================================================

        float* oversampledChannelPointers[2] =
        {
            oversampledBlock.getChannelPointer(0),
            oversampledBlock.getChannelPointer(1)
        };


        juce::AudioBuffer<float> oversampledBuffer(
            oversampledChannelPointers,
            numChannels,
            static_cast<int>(
                oversampledBlock.getNumSamples()
            )
        );


        // ======================================================
        // DOUBLER - OVERSAMPLED
        // ======================================================

        if (doublerMix > 0.0f &&
            doublerAmount > 0.0f)
        {
            doublerProcessor.processBlock(
                oversampledBuffer,
                doublerAmount,
                doublerDetune,
                doublerTiming,
                doublerWidth,
                doublerMix
            );
        }


        // ======================================================
        // HARMONY - OVERSAMPLED
        // ======================================================

        if (harmonyMix > 0.0f &&
            harmonyPitchDetected &&
            hasHarmonyVoice)
        {
            harmonyProcessor.processBlock(
                oversampledBuffer,
                harmonyVoice1,
                harmonyVoice2,
                harmonyVoice3,
                harmonyVoice4,
                harmonyMix,
                harmonyDetectedFrequency,
                harmonyPitchDetected
            );
        }


        // ======================================================
        // CREATIVE FX - OVERSAMPLED
        // ======================================================

        if (creativeFXTypeValue > 0 &&
            creativeFXMix > 0.0f)
        {
            creativeFXProcessor.processBlock(
                oversampledBuffer,
                creativeFXType,
                creativeFXAmount,
                creativeFXMix
            );
        }


        // ======================================================
        // SPACE - OVERSAMPLED
        // ======================================================

        if (spaceType > 0 &&
            spaceMix > 0.0f)
        {
            spaceProcessor.processBlock(
                oversampledBuffer,
                spaceType,
                spaceSize,
                spaceDecay,
                spacePreDelay,
                spaceDamping,
                spaceMix
            );
        }


        // ======================================================
        // DOWNSAMPLE
        // ======================================================

        oversampler->processSamplesDown(
            hostBlock
        );
    }
    else
    {
        // ======================================================
        // OVERSAMPLING OFF
        // ======================================================
        //
        // This is the original host-rate processing path.
        //
        // ======================================================


        // ======================================================
        // DOUBLER - HOST RATE
        // ======================================================

        if (doublerMix > 0.0f &&
            doublerAmount > 0.0f)
        {
            doublerProcessor.processBlock(
                buffer,
                doublerAmount,
                doublerDetune,
                doublerTiming,
                doublerWidth,
                doublerMix
            );
        }


        // ======================================================
        // HARMONY - HOST RATE
        // ======================================================

        if (harmonyMix > 0.0f &&
            harmonyPitchDetected &&
            hasHarmonyVoice)
        {
            harmonyProcessor.processBlock(
                buffer,
                harmonyVoice1,
                harmonyVoice2,
                harmonyVoice3,
                harmonyVoice4,
                harmonyMix,
                harmonyDetectedFrequency,
                harmonyPitchDetected
            );
        }


        // ======================================================
        // CREATIVE FX - HOST RATE
        // ======================================================

        if (creativeFXTypeValue > 0 &&
            creativeFXMix > 0.0f)
        {
            creativeFXProcessor.processBlock(
                buffer,
                creativeFXType,
                creativeFXAmount,
                creativeFXMix
            );
        }


        // ======================================================
        // SPACE - HOST RATE
        // ======================================================

        if (spaceType > 0 &&
            spaceMix > 0.0f)
        {
            spaceProcessor.processBlock(
                buffer,
                spaceType,
                spaceSize,
                spaceDecay,
                spacePreDelay,
                spaceDamping,
                spaceMix
            );
        }
    }

    // ==========================================================
    // GLOBAL MIX
    // ==========================================================
    //
    // Global mix controls the complete Vocal Pro engine.
    //
    // ==========================================================

    if (globalMix < 1.0f)
    {
        const float dryGain =
            1.0f - globalMix;


        for (int channel = 0;
             channel < numChannels;
             ++channel)
        {
            auto* processed =
                buffer.getWritePointer(
                    channel
                );


            const auto* dry =
                dryBuffer.getReadPointer(
                    channel
                );


            for (int sample = 0;
                 sample < numSamples;
                 ++sample)
            {
                processed[sample] =
                    dry[sample] *
                        dryGain
                    +
                    processed[sample] *
                        globalMix;
            }
        }
    }


    // ==========================================================
    // OUTPUT GAIN
    // ==========================================================

    const float outputGain =
        juce::Decibels::decibelsToGain(
            outputDb
        );


    buffer.applyGain(
        outputGain
    );


    // ==========================================================
    // OUTPUT LEVEL METER
    // ==========================================================
    //
    // Measure the final signal AFTER all processing and the
    // OUTPUT gain.
    //
    // ==========================================================

    float outputPeak = 0.0f;


    for (int channel = 0;
         channel < buffer.getNumChannels();
         ++channel)
    {
        outputPeak =
            juce::jmax(
                outputPeak,
                buffer.getMagnitude(
                    channel,
                    0,
                    buffer.getNumSamples()
                )
            );
    }


    const float outputMeterDb =
        outputPeak > 0.000001f
            ? juce::Decibels::gainToDecibels(
                outputPeak
              )
            : -60.0f;


    outputLevelDb.store(
        juce::jlimit(
            -60.0f,
            0.0f,
            outputMeterDb
        )
    );


    // ======================================================
    // SAFETY
    // ======================================================

    // Intentionally no additional gain processing here.
}


//==============================================================================
// SETTINGS - PROCESSING ENABLE
//==============================================================================

void OfforVocalProAudioProcessor::setProcessingEnabled(
    bool enabled)
{
    processingEnabled.store(
        enabled);
}

//==============================================================================

bool OfforVocalProAudioProcessor::isProcessingEnabled() const
{
    return processingEnabled.load();
}


// ==========================================================
// SET OVERSAMPLING MODE
// ==========================================================

void OfforVocalProAudioProcessor::setOversamplingMode(
    const juce::String& mode)
{
    // ==========================================================
    // VALIDATE
    // ==========================================================

    if (mode != "Off"
        && mode != "2X"
        && mode != "4X"
        && mode != "8X")
    {
        return;
    }


    // ==========================================================
    // NOTHING TO DO IF ALREADY ACTIVE
    // ==========================================================

    if (oversamplingMode == mode)
        return;


    // ==========================================================
    // UPDATE SETTING
    // ==========================================================

    oversamplingMode = mode;


    // ==========================================================
    // RECONFIGURE DSP
    // ==========================================================
    //
    // Changing oversampling requires:
    //
    //     1. Creating a new oversampling engine
    //     2. Reallocating its internal buffers
    //     3. Re-preparing the creative DSP
    //     4. Updating latency
    //
    // These operations must NOT happen simultaneously with
    // processBlock().
    //
    // ==========================================================

    const juce::SpinLock::ScopedLockType lock(
        dspConfigurationLock);

    configureOversampling(
        oversamplingMode);
}


// ==========================================================
// GET OVERSAMPLING MODE
// ==========================================================

juce::String
OfforVocalProAudioProcessor::getOversamplingMode() const
{
    return oversamplingMode;
}


//==============================================================================
// SETTINGS - PROCESSING QUALITY
//==============================================================================

void OfforVocalProAudioProcessor::setProcessingQuality(
    const juce::String& quality
)
{
    processingQuality = quality;

    //==============================================================
    // DOUBLER
    //==============================================================

    DoublerProcessor::ProcessingQuality doublerQuality =
        DoublerProcessor::ProcessingQuality::High;

    //==============================================================
    // HARMONY
    //==============================================================

    HarmonyProcessor::ProcessingQuality harmonyQuality =
        HarmonyProcessor::ProcessingQuality::High;

    //==============================================================
    // CREATIVE FX
    //==============================================================

    CreativeFXProcessor::ProcessingQuality creativeFXQuality =
        CreativeFXProcessor::ProcessingQuality::High;

    //==============================================================

    if (quality == "Low")
    {
        doublerQuality =
            DoublerProcessor::ProcessingQuality::Low;

        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::Low;

        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::Low;
    }
    else if (quality == "Medium")
    {
        doublerQuality =
            DoublerProcessor::ProcessingQuality::Medium;

        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::Medium;

        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::Medium;
    }
    else if (quality == "High")
    {
        doublerQuality =
            DoublerProcessor::ProcessingQuality::High;

        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::High;

        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::High;
    }
    else if (quality == "Ultra")
    {
        doublerQuality =
            DoublerProcessor::ProcessingQuality::Ultra;

        harmonyQuality =
            HarmonyProcessor::ProcessingQuality::Ultra;

        creativeFXQuality =
            CreativeFXProcessor::ProcessingQuality::Ultra;
    }

    //==============================================================
    // APPLY REAL DSP QUALITY
    //==============================================================

    doublerProcessor.setProcessingQuality(
        doublerQuality
    );

    harmonyProcessor.setProcessingQuality(
        harmonyQuality
    );

    creativeFXProcessor.setProcessingQuality(
        creativeFXQuality
    );
}

//==============================================================================

juce::String OfforVocalProAudioProcessor::getProcessingQuality() const
{
    return processingQuality;
}


//==============================================================================
// SETTINGS - CPU MODE
//==============================================================================

void OfforVocalProAudioProcessor::setCPUMode(
    const juce::String& mode
)
{
    cpuMode = mode;

    //==============================================================
    // DOUBLER
    //==============================================================

    DoublerProcessor::CPUMode doublerMode =
        DoublerProcessor::CPUMode::Balanced;

    //==============================================================
    // HARMONY
    //==============================================================

    HarmonyProcessor::CPUMode harmonyMode =
        HarmonyProcessor::CPUMode::Balanced;

    //==============================================================
    // CREATIVE FX
    //==============================================================

    CreativeFXProcessor::CPUMode creativeFXMode =
        CreativeFXProcessor::CPUMode::Balanced;

    //==============================================================

    if (mode == "Low CPU")
    {
        doublerMode =
            DoublerProcessor::CPUMode::LowCPU;

        harmonyMode =
            HarmonyProcessor::CPUMode::LowCPU;

        creativeFXMode =
            CreativeFXProcessor::CPUMode::LowCPU;
    }
    else if (mode == "Balanced")
    {
        doublerMode =
            DoublerProcessor::CPUMode::Balanced;

        harmonyMode =
            HarmonyProcessor::CPUMode::Balanced;

        creativeFXMode =
            CreativeFXProcessor::CPUMode::Balanced;
    }
    else if (mode == "Performance")
    {
        doublerMode =
            DoublerProcessor::CPUMode::Performance;

        harmonyMode =
            HarmonyProcessor::CPUMode::Performance;

        creativeFXMode =
            CreativeFXProcessor::CPUMode::Performance;
    }

    //==============================================================
    // APPLY REAL DSP CPU MODE
    //==============================================================

    doublerProcessor.setCPUMode(
        doublerMode
    );

    harmonyProcessor.setCPUMode(
        harmonyMode
    );

    creativeFXProcessor.setCPUMode(
        creativeFXMode
    );
}

//==============================================================================

juce::String OfforVocalProAudioProcessor::getCPUMode() const
{
    return cpuMode;
}


// ==========================================================
// PLUGIN INFORMATION
// ==========================================================

const juce::String
OfforVocalProAudioProcessor::
getName() const
{
    return JucePlugin_Name;
}


double
OfforVocalProAudioProcessor::
getTailLengthSeconds() const
{
    return 0.0;
}


// ==========================================================
// PROGRAMS
// ==========================================================

int
OfforVocalProAudioProcessor::
getNumPrograms()
{
    return 1;
}


int
OfforVocalProAudioProcessor::
getCurrentProgram()
{
    return 0;
}


void
OfforVocalProAudioProcessor::
setCurrentProgram(
    int index)
{
    juce::ignoreUnused(
        index
    );
}


const juce::String
OfforVocalProAudioProcessor::
getProgramName(
    int index)
{
    juce::ignoreUnused(
        index
    );

    return {};
}


void
OfforVocalProAudioProcessor::
changeProgramName(
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused(
        index,
        newName
    );
}


// ==========================================================
// STATE SAVE
// ==========================================================
//
// Saves:
//
//     1. All APVTS parameters
//     2. Selected UI theme
//     3. All custom theme colours
//
// Everything is stored inside the same ValueTree/XML state
// that the DAW already saves.
//
// This means the theme follows:
//
//     - DAW projects
//     - Plugin state
//     - Preset/state recall
//
// ==========================================================

void
OfforVocalProAudioProcessor::
getStateInformation(
    juce::MemoryBlock& destData)
{
    // ======================================================
    // COPY CURRENT PARAMETER STATE
    // ======================================================

    auto state =
        apvts.copyState();


    // ======================================================
    // SAVE SELECTED THEME
    // ======================================================

    state.setProperty(
        "OFFOR_THEME",
        ThemeManager::get().getThemeName(),
        nullptr
    );


    // ======================================================
    // SAVE CUSTOM COLOURS
    // ======================================================
    //
    // We save every custom colour individually.
    //
    // ARGB is used because it preserves the complete JUCE
    // Colour value including alpha.
    //
    // ======================================================

    const juce::StringArray colourIds =
    {
        "background",
        "panel",
        "panel2",
        "border",
        "text",
        "muted",
        "accent",
        "accentDark",
        "display",
        "knob",
        "knobHighlight",
        "meter",
        "meterBackground",
        "success",
        "warning"
    };


    for (const auto& colourId : colourIds)
    {
        const auto colour =
            ThemeManager::get().getCustomColour(
                colourId
            );


        state.setProperty(
            "OFFOR_COLOUR_" + colourId,
            static_cast<int64_t>(
                colour.getARGB()
            ),
            nullptr
        );
    }


    // ======================================================
    // CREATE XML
    // ======================================================

    std::unique_ptr<juce::XmlElement> xml =
        state.createXml();


    // ======================================================
    // WRITE STATE TO HOST
    // ======================================================

    if (xml != nullptr)
    {
        copyXmlToBinary(
            *xml,
            destData
        );
    }
}

// ==========================================================
// STATE RESTORE
// ==========================================================
//
// Restores:
//
//     1. All APVTS parameters
//     2. Selected UI theme
//     3. Custom theme colours
//
// Older saved states that do not contain theme properties
// are still supported.
//
// ==========================================================

void
OfforVocalProAudioProcessor::
setStateInformation(
    const void* data,
    int sizeInBytes)
{
    // ======================================================
    // READ XML
    // ======================================================

    std::unique_ptr<juce::XmlElement> xml =
        getXmlFromBinary(
            data,
            sizeInBytes
        );


    if (xml == nullptr)
        return;


    // ======================================================
    // VERIFY ROOT
    // ======================================================

    if (!xml->hasTagName(
            apvts.state.getType()
        ))
    {
        return;
    }


    // ======================================================
    // CONVERT XML → VALUETREE
    // ======================================================

    const juce::ValueTree restoredState =
        juce::ValueTree::fromXml(
            *xml
        );


    if (!restoredState.isValid())
        return;


    // ======================================================
    // RESTORE APVTS
    // ======================================================
    //
    // This remains exactly the same principle as your
    // original implementation.
    //
    // All audio parameters are restored here.
    //
    // ======================================================

    apvts.replaceState(
        restoredState
    );


    // ======================================================
    // RESTORE THEME
    // ======================================================
    //
    // If this property does not exist, this is an older
    // saved state and we simply keep the current/default
    // theme.
    //
    // ======================================================

    if (restoredState.hasProperty(
            "OFFOR_THEME"))
    {
        const juce::String savedTheme =
            restoredState
                .getProperty(
                    "OFFOR_THEME"
                )
                .toString();


        if (savedTheme.isNotEmpty())
        {
            ThemeManager::get().setTheme(
                savedTheme
            );
        }
    }


    // ======================================================
    // RESTORE CUSTOM COLOURS
    // ======================================================
    //
    // Restore every saved custom colour.
    //
    // We deliberately do this AFTER restoring the theme.
    //
    // If the saved theme is "Custom", these colours become
    // the active Custom palette.
    //
    // ======================================================

    const juce::StringArray colourIds =
    {
        "background",
        "panel",
        "panel2",
        "border",
        "text",
        "muted",
        "accent",
        "accentDark",
        "display",
        "knob",
        "knobHighlight",
        "meter",
        "meterBackground",
        "success",
        "warning"
    };


    for (const auto& colourId : colourIds)
    {
        const juce::String propertyName =
            "OFFOR_COLOUR_" + colourId;


        if (!restoredState.hasProperty(
                propertyName))
        {
            continue;
        }


        const int64_t argb =
            static_cast<int64_t>(
                restoredState.getProperty(
                    propertyName
                )
            );


        ThemeManager::get().setCustomColourWithoutChangingTheme(
            colourId,
            juce::Colour(
                static_cast<juce::uint32>(
                    argb
                )
            )
        );
    }


    // ======================================================
    // RESTORE THEME ONE MORE TIME
    // ======================================================
    //
    // setCustomColour() intentionally switches the
    // ThemeManager to "Custom".
    //
    // Therefore we restore the saved theme AFTER loading
    // the colours so that a saved built-in theme such as
    // "Midnight Blue" does NOT accidentally become Custom.
    //
    // ======================================================

    if (restoredState.hasProperty(
            "OFFOR_THEME"))
    {
        const juce::String savedTheme =
            restoredState
                .getProperty(
                    "OFFOR_THEME"
                )
                .toString();


        if (savedTheme.isNotEmpty())
        {
            ThemeManager::get().setTheme(
                savedTheme
            );
        }
    }


    // ======================================================
    // RESET TARGET CACHE
    // ======================================================

    previousTargetMidiNote =
        60.0;


    smoothedTargetMidiNote =
        60.0;


    targetMidiNote.store(
        60.0
    );


    targetPitchClass.store(
        0
    );
}


// ==========================================================
// START LICENSE SESSION
// ==========================================================
//
// IMPORTANT:
//
// This function DOES NOT perform the network request itself.
//
// It creates and starts a JUCE-managed background thread.
//
// The previous implementation:
//
//     std::thread(...).detach();
//
// has deliberately been removed.
//
// ==========================================================

bool
OfforVocalProAudioProcessor::
startLicenseSession()
{
    // ======================================================
    // PREVENT STARTING THE SAME SESSION MORE THAN ONCE
    // ======================================================

    bool expected = false;


    if (!licenseSessionStarted.compare_exchange_strong(
            expected,
            true))
    {
        return licenseAllowed.load();
    }


    // ======================================================
    // ALREADY ACTIVATED?
    // ======================================================
    //
    // No free use needs to be consumed.
    //
    // ======================================================

    if (licenseManager.isActivated())
    {
        licenseAllowed.store(true);

        return true;
    }


    // ======================================================
    // CREATE BACKGROUND LICENSE THREAD
    // ======================================================
    //
    // The thread object is owned by this processor.
    //
    // It will be safely stopped by the processor destructor.
    //
    // ======================================================

    licenseSessionThread =
        std::make_unique<LicenseSessionThread>(
            *this
        );


    // ======================================================
    // START THREAD
    // ======================================================
    //
    // This does NOT block the message thread.
    //
    // Network operations happen inside:
    //
    //     LicenseSessionThread::run()
    //
    // ======================================================

    licenseSessionThread->startThread();


    // ======================================================
    // SERVER REQUEST IS RUNNING
    // ======================================================
    //
    // Audio processing remains blocked until the background
    // thread receives the server response and stores the
    // result in licenseAllowed.
    //
    // ======================================================

    return false;
}


// ==========================================================
// ACTIVATE LICENSE
// ==========================================================
//
// Called from the editor/message thread when the user enters
// a purchased license key.
//
// The network operation is NOT performed from processBlock().
//
// ==========================================================

bool
OfforVocalProAudioProcessor::
activateLicense(
    const juce::String& licenseKey)
{
    const bool activated =
        licenseManager.activate(
            licenseKey.trim()
        );


    if (activated)
    {
        // ==================================================
        // LICENSE IS NOW ACTIVE
        // ==================================================

        licenseAllowed.store(true);


        // ==================================================
        // THIS PROCESSOR INSTANCE IS NOW AUTHORIZED.
        //
        // Do not allow another free-use session to start.
        // ==================================================

        licenseSessionStarted.store(true);
    }


    return activated;
}


// ==========================================================
// CREATE EDITOR
// ==========================================================

juce::AudioProcessorEditor*
OfforVocalProAudioProcessor::
createEditor()
{
    // ======================================================
    // START LICENSE SESSION
    // ======================================================
    //
    // createEditor() normally runs on JUCE's message/UI
    // thread.
    //
    // startLicenseSession() only starts the background
    // JUCE thread. It does NOT perform synchronous HTTP
    // work on this thread.
    //
    // ======================================================

    startLicenseSession();


    return new OfforVocalProAudioProcessorEditor(
        *this
    );
}


// ==========================================================
// CREATE PLUGIN INSTANCE
// ==========================================================

juce::AudioProcessor*
JUCE_CALLTYPE
createPluginFilter()
{
    return new OfforVocalProAudioProcessor();
}