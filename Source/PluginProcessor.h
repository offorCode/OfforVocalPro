#pragma once

#include <JuceHeader.h>

#include "Audio/PitchDetector.h"
#include "Audio/PitchCorrector.h"
#include "Audio/DoublerProcessor.h"
#include "Audio/HarmonyProcessor.h"
#include "Audio/CreativeFXProcessor.h"
#include "Audio/SpaceProcessor.h"

// ==========================================================
// LICENSING
// ==========================================================
//
// Offor Vocal Pro uses the central OFFOR license server.
//
// Vocal Pro has:
//     10 free uses
//
// Stem Splitter / Sampler remain independent products.
//
// LicenseManager communicates with:
//
//     /api/v1/license/register
//     /api/v1/license/use
//     /api/v1/license/activate
//
// IMPORTANT:
//
// We do NOT perform license checks from processBlock().
//
// processBlock() runs continuously on the audio thread.
// Network operations must NEVER happen there.
//
// Instead, a dedicated JUCE background thread handles:
//     1. Installation registration
//     2. One free-use session check
//
// The audio thread only reads the atomic licenseAllowed flag.
// ==========================================================

#include "Licensing/LicenseManager.h"


class OfforVocalProAudioProcessor
    : public juce::AudioProcessor
{
public:

    // ==========================================================
    // CONSTRUCTOR / DESTRUCTOR
    // ==========================================================

    OfforVocalProAudioProcessor();

    ~OfforVocalProAudioProcessor() override;


    // ==========================================================
    // AUDIO PROCESSOR
    // ==========================================================

    void prepareToPlay(
        double sampleRate,
        int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(
        const BusesLayout& layouts) const override;

    void processBlock(
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&) override;


    // ==========================================================
    // EDITOR
    // ==========================================================

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override
    {
        return true;
    }


    // ==========================================================
    // PLUGIN INFORMATION
    // ==========================================================

    const juce::String getName() const override;

    bool acceptsMidi() const override
    {
        return false;
    }

    bool producesMidi() const override
    {
        return false;
    }

    bool isMidiEffect() const override
    {
        return false;
    }

    double getTailLengthSeconds() const override;


    // ==========================================================
    // PROGRAMS
    // ==========================================================

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(
        int index) override;

    const juce::String getProgramName(
        int index) override;

    void changeProgramName(
        int index,
        const juce::String& newName) override;


    // ==========================================================
    // STATE
    // ==========================================================

    void getStateInformation(
        juce::MemoryBlock& destData) override;

    void setStateInformation(
        const void* data,
        int sizeInBytes) override;


    // ==========================================================
    // APVTS
    // ==========================================================

    juce::AudioProcessorValueTreeState apvts;


    // ==========================================================
    // LICENSING
    // ==========================================================
    //
    // These functions allow PluginEditor to display the current
    // license state without directly accessing LicenseManager.
    //
    // ==========================================================

    bool isLicenseActivated() const
    {
        return licenseManager.isActivated();
    }


    int getFreeUsesRemaining() const
    {
        return licenseManager.getFreeUsesRemaining();
    }


    int getServerFreeUses() const
    {
        return licenseManager.getServerFreeUses();
    }


    int getServerFreeUsesLimit() const
    {
        return licenseManager.getServerFreeUsesLimit();
    }


    juce::String getLicenseKey() const
    {
        return licenseManager.getStoredLicense();
    }


    juce::String getInstallationId() const
    {
        return licenseManager.getInstallationId();
    }

    // ==========================================================
    // LICENSE SESSION STATE
    // ==========================================================
    //
    // These are read by the editor only.
    // The actual license state remains owned by the processor.
    //
    // licenseSessionStarted:
    //     false = background license check has not completed
    //     true  = license session has started
    //
    // licenseAllowed:
    //     false = processing is blocked
    //     true  = processing is allowed
    //
    // ==========================================================

    bool isLicenseSessionStarted() const
    {
        return licenseSessionStarted.load();
    }

    bool isLicenseAllowed() const
    {
        return licenseAllowed.load();
    }


    // ==========================================================
    // START LICENSE SESSION
    // ==========================================================
    //
    // This starts ONE license session.
    //
    // For an unactivated installation:
    //
    //     register installation
    //              ↓
    //          /use request
    //              ↓
    //       server consumes 1 use
    //
    // For an activated installation:
    //
    //     processing is allowed immediately
    //
    // IMPORTANT:
    //
    // This function itself does not perform network operations.
    // The network work is performed by LicenseSessionThread.
    //
    // It must NOT be called from processBlock().
    //
    // ==========================================================

    bool startLicenseSession();


    // ==========================================================
    // ACTIVATE LICENSE
    // ==========================================================
    //
    // Called by the license/settings UI.
    //
    // ==========================================================

    bool activateLicense(
        const juce::String& licenseKey);


    // ==========================================================
    // STABLE PARAMETER IDS
    //
    // These IDs should now be treated as release IDs.
    // Do not casually rename them after release.
    // ==========================================================

    // ----------------------------------------------------------
    // GLOBAL
    // ----------------------------------------------------------

    static constexpr const char* PARAM_BYPASS =
        "global.bypass";

    static constexpr const char* PARAM_INPUT =
        "global.input";

    static constexpr const char* PARAM_OUTPUT =
        "global.output";

    static constexpr const char* PARAM_MIX =
        "global.mix";


    // ----------------------------------------------------------
    // TUNER
    // ----------------------------------------------------------

    static constexpr const char* PARAM_TUNER_KEY =
        "tuner.key";

    static constexpr const char* PARAM_TUNER_SCALE =
        "tuner.scale";

    static constexpr const char* PARAM_TUNER_MODE =
        "tuner.mode";

    static constexpr const char* PARAM_TUNER_RETUNE =
        "tuner.retune";

    static constexpr const char* PARAM_TUNER_SMOOTH =
        "tuner.smooth";

    static constexpr const char* PARAM_TUNER_FORMANT =
        "tuner.formant";

    static constexpr const char* PARAM_TUNER_MIX =
        "tuner.mix";


    // ----------------------------------------------------------
    // DOUBLER
    // ----------------------------------------------------------

    static constexpr const char* PARAM_DOUBLER_AMOUNT =
        "doubler.amount";

    static constexpr const char* PARAM_DOUBLER_DETUNE =
        "doubler.detune";

    static constexpr const char* PARAM_DOUBLER_TIMING =
        "doubler.timing";

    static constexpr const char* PARAM_DOUBLER_WIDTH =
        "doubler.width";

    static constexpr const char* PARAM_DOUBLER_MIX =
        "doubler.mix";


    // ----------------------------------------------------------
    // HARMONY
    // ----------------------------------------------------------

    static constexpr const char* PARAM_HARMONY_VOICE1 =
        "harmony.voice1";

    static constexpr const char* PARAM_HARMONY_VOICE2 =
        "harmony.voice2";

    static constexpr const char* PARAM_HARMONY_VOICE3 =
        "harmony.voice3";

    static constexpr const char* PARAM_HARMONY_VOICE4 =
        "harmony.voice4";

    static constexpr const char* PARAM_HARMONY_MIX =
        "harmony.mix";


    // ----------------------------------------------------------
    // CREATIVE FX
    // ----------------------------------------------------------

    static constexpr const char* PARAM_FX_TYPE =
        "fx.type";

    static constexpr const char* PARAM_FX_AMOUNT =
        "fx.amount";

    static constexpr const char* PARAM_FX_MIX =
        "fx.mix";


    // ----------------------------------------------------------
    // SPACE
    // ----------------------------------------------------------

    static constexpr const char* PARAM_SPACE_TYPE =
        "space.type";

    static constexpr const char* PARAM_SPACE_SIZE =
        "space.size";

    static constexpr const char* PARAM_SPACE_DECAY =
        "space.decay";

    static constexpr const char* PARAM_SPACE_PREDELAY =
        "space.predelay";

    static constexpr const char* PARAM_SPACE_DAMPING =
        "space.damping";

    static constexpr const char* PARAM_SPACE_MIX =
        "space.mix";


    // ==========================================================
    // PITCH INFORMATION FOR UI
    // ==========================================================

    double getDetectedFrequency() const
    {
        return pitchDetector.getFrequencyHz();
    }

    double getDetectedMidiNote() const
    {
        return pitchDetector.getMidiNote();
    }

    double getDetectedCents() const
    {
        return pitchDetector.getCents();
    }

    double getPitchConfidence() const
    {
        return pitchDetector.getConfidence();
    }

    bool hasDetectedPitch() const
    {
        return pitchDetector.isPitchDetected();
    }


    // ==========================================================
    // TARGET NOTE INFORMATION
    // ==========================================================

    double getTargetMidiNote() const
    {
        return targetMidiNote.load();
    }

    int getTargetPitchClass() const
    {
        return targetPitchClass.load();
    }


    // ==========================================================
    // LEVEL METERS
    // ==========================================================
    //
    // Audio thread -> writes
    // GUI timer    -> reads
    //
    // Atomics make this safe without using a mutex or touching
    // GUI objects from the audio thread.
    //

    float getInputLevelDb() const
    {
        return inputLevelDb.load();
    }

    float getOutputLevelDb() const
    {
        return outputLevelDb.load();
    }


        //==========================================================================
    // SETTINGS
    //==========================================================================
    //
    // These are application/plugin-engine settings rather than audio
    // automation parameters, so they are intentionally NOT added to APVTS.
    //
    // APVTS remains responsible for actual plugin parameters:
    //
    //     Input
    //     Output
    //     Mix
    //     Tuner
    //     Doubler
    //     Harmony
    //     Creative FX
    //     Space
    //
    // SettingsPanel uses these functions for engine-level preferences.
    //
    //==========================================================================

    void setProcessingEnabled(bool enabled);
    bool isProcessingEnabled() const;

    void setOversamplingMode(const juce::String& mode);
    juce::String getOversamplingMode() const;

    void setProcessingQuality(const juce::String& quality);
    juce::String getProcessingQuality() const;

    void setCPUMode(const juce::String& mode);
    juce::String getCPUMode() const;


private:

    // ==========================================================
    // LICENSE SESSION THREAD
    // ==========================================================
    //
    // IMPORTANT:
    //
    // The old implementation used:
    //
    //     std::thread(...).detach();
    //
    // That is unsafe because the detached thread could still
    // access the AudioProcessor after the processor was destroyed.
    //
    // This JUCE Thread belongs to the processor and is stopped
    // before the processor is destroyed.
    //
    // ==========================================================

    class LicenseSessionThread
        : public juce::Thread
    {
    public:

        explicit LicenseSessionThread(
            OfforVocalProAudioProcessor& ownerProcessor);

        ~LicenseSessionThread() override = default;


    private:

        void run() override;


        OfforVocalProAudioProcessor& owner;


        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            LicenseSessionThread
        )
    };


    // ==========================================================
    // SCALE TYPES
    // ==========================================================

    enum class ScaleType
    {
        Chromatic = 0,
        Major,
        Minor
    };


    // ==========================================================
    // TUNER MODES
    // ==========================================================

    enum class TunerMode
    {
        Natural = 0,
        Modern,
        HardTune
    };


    // ==========================================================
    // PARAMETER CREATION
    // ==========================================================

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();


    // ==========================================================
    // TARGET NOTE SELECTOR
    // ==========================================================

    double selectTargetMidiNote(
        double detectedMidiNote,
        int key,
        ScaleType scale) const;


    bool isPitchClassInScale(
        int pitchClass,
        int key,
        ScaleType scale) const;


    int findNearestScalePitchClass(
        int detectedPitchClass,
        int key,
        ScaleType scale) const;


    // ==========================================================
    // NOTE HELPERS
    // ==========================================================

    static double midiToFrequency(
        double midiNote);

    // ==========================================================
    // OVERSAMPLING ENGINE
    // ==========================================================
    //
    // OFFOR Vocal Pro keeps:
    //
    //     Pitch Detection
    //     Pitch Correction
    //
    // at the host sample rate.
    //
    // The creative processing chain:
    //
    //     Doubler
    //     Harmony
    //     Creative FX
    //     Space
    //
    // can then run at:
    //
    //     Off -> 1X
    //     2X  -> 2X host rate
    //     4X  -> 4X host rate
    //     8X  -> 8X host rate
    //
    // JUCE's Oversampling class handles the anti-aliasing
    // filters and sample-rate conversion.
    //
    // ==========================================================

    std::unique_ptr<juce::dsp::Oversampling<float>>
        oversampler;


    // ==========================================================
    // OVERSAMPLING CONFIGURATION
    // ==========================================================
    //
    // This function is called when the oversampling setting changes
    // or when prepareToPlay() receives a new host sample rate.
    //
    // It is NEVER called directly from processBlock().
    //
    // ==========================================================

    void configureOversampling(
        const juce::String& mode);


    // ==========================================================
    // OVERSAMPLING FACTOR
    // ==========================================================
    //
    // Returns:
    //
    //     Off -> 1
    //     2X  -> 2
    //     4X  -> 4
    //     8X  -> 8
    //
    // ==========================================================

    int getOversamplingFactorFromMode(
        const juce::String& mode) const;


    // ==========================================================
    // DSP CONFIGURATION LOCK
    // ==========================================================
    //
    // Changing oversampling requires rebuilding the JUCE
    // Oversampling object and re-preparing the downstream DSP.
    //
    // That must not happen simultaneously with processBlock().
    //
    // The GUI/configuration side takes this lock while rebuilding.
    //
    // The audio thread uses ScopedTryLock instead of waiting.
    //
    // If the lock is unavailable, processBlock() simply leaves
    // that block untouched.
    //
    // This prevents the real-time audio thread from blocking.
    //
    // ==========================================================

    juce::SpinLock dspConfigurationLock;


    // ==========================================================
    // LICENSING
    // ==========================================================

    //
    // One persistent LicenseManager belongs to this plugin
    // processor instance.
    //

    LicenseManager licenseManager;


    // ==========================================================
    // LICENSE BACKGROUND THREAD
    // ==========================================================
    //
    // Owned by the processor.
    //
    // IMPORTANT:
    //
    // This is declared AFTER licenseManager so that during normal
    // destruction the thread member is destroyed before the
    // LicenseManager member.
    //
    // The processor destructor explicitly stops the thread first.
    //
    // ==========================================================

    std::unique_ptr<LicenseSessionThread>
        licenseSessionThread;


    // ==========================================================
    // PROCESSING
    // ==========================================================

    PitchDetector pitchDetector;

    PitchCorrector pitchCorrector;

    DoublerProcessor doublerProcessor;

    HarmonyProcessor harmonyProcessor;

    CreativeFXProcessor creativeFXProcessor;

    SpaceProcessor spaceProcessor;


    // ==========================================================
    // DRY BUFFER
    // ==========================================================

    juce::AudioBuffer<float> dryBuffer;


    // ==========================================================
    // PROCESSOR STATE
    // ==========================================================

    double currentSampleRate = 44100.0;

    int currentBlockSize = 0;


    // ==========================================================
    // LICENSE SESSION STATE
    // ==========================================================
    //
    // This prevents accidental repeated consumption of a free
    // use during the same plugin session.
    //
    // false = no usage has been consumed for this session.
    // true  = this session has already been authorized/started.
    //
    // licenseAllowed:
    //
    // false = audio processing is blocked.
    // true  = audio processing is allowed.
    //
    // ==========================================================

    std::atomic<bool> licenseSessionStarted
    {
        false
    };

    std::atomic<bool> licenseAllowed
    {
        false
    };


    // ==========================================================
    // TARGET NOTE STATE
    // ==========================================================

    std::atomic<double> targetMidiNote
    {
        60.0
    };

    std::atomic<int> targetPitchClass
    {
        0
    };


    // ==========================================================
    // LEVEL METER STATE
    // ==========================================================
    //
    // Audio thread -> writes
    // GUI timer    -> reads
    //
    // We store dB values because LevelMeter already understands
    // dB directly.
    //

    std::atomic<float> inputLevelDb
    {
        -60.0f
    };

    std::atomic<float> outputLevelDb
    {
        -60.0f
    };


    //==========================================================================
    // SETTINGS STATE
    //==========================================================================

    std::atomic<bool> processingEnabled{true};

    juce::String oversamplingMode = "2X";
    juce::String processingQuality = "High";
    juce::String cpuMode = "Balanced";


    // ==========================================================
    // PREVIOUS TARGET
    // ==========================================================

    double previousTargetMidiNote = 60.0;

    double smoothedTargetMidiNote = 60.0;


    // ==========================================================
    // JUCE
    // ==========================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        OfforVocalProAudioProcessor
    )
};