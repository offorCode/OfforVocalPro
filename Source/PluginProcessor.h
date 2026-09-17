#pragma once

#include <JuceHeader.h>

#include "Audio/PitchDetector.h"
#include "Audio/PitchCorrector.h"
#include "Audio/DoublerProcessor.h"
#include "Audio/HarmonyProcessor.h"
#include "Audio/CreativeFXProcessor.h"
#include "Audio/SpaceProcessor.h"


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
    // These values are written by the audio thread and read by
    // the editor timer. Atomics make this safe without using a
    // mutex or touching GUI objects from the audio thread.
    //

    float getInputLevelDb() const
    {
        return inputLevelDb.load();
    }

    float getOutputLevelDb() const
    {
        return outputLevelDb.load();
    }


private:

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

    std::atomic<float> inputLevelDb { -60.0f };
    std::atomic<float> outputLevelDb { -60.0f };


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