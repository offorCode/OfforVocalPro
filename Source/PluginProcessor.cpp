#include "PluginProcessor.h"
#include "PluginEditor.h"

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

void
OfforVocalProAudioProcessor::
prepareToPlay(
    double newSampleRate,
    int samplesPerBlock)
{
    currentSampleRate = newSampleRate;
    currentBlockSize = samplesPerBlock;


    pitchDetector.prepare(
        newSampleRate,
        samplesPerBlock
    );


    pitchCorrector.prepare(
        newSampleRate,
        samplesPerBlock,
        2
    );


    doublerProcessor.prepare(
        newSampleRate,
        samplesPerBlock,
        2
    );


    harmonyProcessor.prepare(
        newSampleRate,
        samplesPerBlock,
        2
    );


    creativeFXProcessor.prepare(
        newSampleRate,
        samplesPerBlock,
        2
    );


    spaceProcessor.prepare(
        newSampleRate,
        samplesPerBlock,
        2
    );


    dryBuffer.setSize(
        getTotalNumInputChannels(),
        samplesPerBlock
    );


    dryBuffer.clear();


    targetMidiNote.store(60.0);

    targetPitchClass.store(0);


    previousTargetMidiNote = 60.0;

    smoothedTargetMidiNote = 60.0;
}


// ==========================================================
// RELEASE RESOURCES
// ==========================================================

void
OfforVocalProAudioProcessor::
releaseResources()
{
    pitchDetector.reset();

    pitchCorrector.reset();

    doublerProcessor.reset();

    harmonyProcessor.reset();

    creativeFXProcessor.reset();

    spaceProcessor.reset();


    dryBuffer.setSize(
        0,
        0
    );
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


    // ======================================================
    // DOUBLER V1
    // ======================================================

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


    // ==========================================================
    // HARMONY
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


    // ----------------------------------------------------------
    // Calculate a scale-aware harmony interval.
    //
    // Chromatic mode retains fixed intervals.
    //
    // Major/Minor mode follows the selected scale.
    // ----------------------------------------------------------

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
                        harmonyKey
                        +
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
    // CALCULATE ALL FOUR VOICES
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
    // PROCESS HARMONY
    // ==========================================================

    if (harmonyMix > 0.0f &&
        harmonyPitchDetected)
    {
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
            if (std::abs(interval) >
                0.01f)
            {
                hasHarmonyVoice =
                    true;

                break;
            }
        }


        if (hasHarmonyVoice)
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
    }


    // ==========================================================
    // CREATIVE VOCAL FX
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


    // ==========================================================
    // VOCAL SPACE
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

void
OfforVocalProAudioProcessor::
getStateInformation(
    juce::MemoryBlock& destData)
{
    auto state =
        apvts.copyState();


    std::unique_ptr<juce::XmlElement> xml =
        state.createXml();


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

void
OfforVocalProAudioProcessor::
setStateInformation(
    const void* data,
    int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml =
        getXmlFromBinary(
            data,
            sizeInBytes
        );


    if (xml == nullptr)
        return;


    if (!xml->hasTagName(
            apvts.state.getType()
        ))
    {
        return;
    }


    const juce::ValueTree restoredState =
        juce::ValueTree::fromXml(
            *xml
        );


    if (!restoredState.isValid())
        return;


    apvts.replaceState(
        restoredState
    );


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