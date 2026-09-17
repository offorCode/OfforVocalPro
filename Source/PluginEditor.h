#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class OfforVocalProAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:

    explicit OfforVocalProAudioProcessorEditor(
        OfforVocalProAudioProcessor& processor);

    ~OfforVocalProAudioProcessorEditor() override;

    void paint(
        juce::Graphics& g) override;

    void resized() override;

private:

    // ==========================================================
    // MODULES
    // ==========================================================

    enum class Module
    {
        tuner,
        doubler,
        harmony,
        creativeFx,
        space
    };

    Module currentModule = Module::tuner;

    void selectModule(Module module);

    void updateModuleVisibility();

    // ==========================================================
    // CUSTOM MODULE BUTTON
    // ==========================================================

    class ModuleButton : public juce::TextButton
    {
    public:

        ModuleButton();

        void setIcon(const juce::Image& image);

        void setModuleName(const juce::String& name);

        void setSelected(bool selected);

        void paintButton(
            juce::Graphics& g,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;

    private:

        juce::Image icon;
        juce::String moduleName;
        bool selected = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            ModuleButton
        )
    };

    ModuleButton tunerButton;
    ModuleButton doublerButton;
    ModuleButton harmonyButton;
    ModuleButton fxButton;
    ModuleButton spaceButton;

    // ==========================================================
    // HEADER
    // ==========================================================

    juce::Image logoImage;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label versionLabel;

    juce::TextButton settingsButton;
    juce::ToggleButton bypassButton;

    juce::Image settingsIcon;

    juce::Image radioTunerIcon;
    juce::Image doublerIcon;
    juce::Image harmonyIcon;
    juce::Image fxIcon;
    juce::Image spaceIcon;

    // ==========================================================
    // PROFESSIONAL HARDWARE KNOB
    // ==========================================================

    class ProfessionalKnob : public juce::Slider
    {
    public:

        ProfessionalKnob();

        void paint(juce::Graphics& g) override;

        void mouseEnter(
            const juce::MouseEvent& event) override;

        void mouseExit(
            const juce::MouseEvent& event) override;

        void mouseDown(
            const juce::MouseEvent& event) override;

        void mouseUp(
            const juce::MouseEvent& event) override;

    private:

        bool isActive = false;

        void repaintKnob();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            ProfessionalKnob
        )
    };

    // ==========================================================
    // GLOBAL CONTROLS
    // ==========================================================

    // juce::Slider inputSlider;
    // juce::Slider globalMixSlider;
    // juce::Slider outputSlider;

    ProfessionalKnob inputSlider;
    ProfessionalKnob globalMixSlider;
    ProfessionalKnob outputSlider;

    juce::Label inputLabel;
    juce::Label globalMixLabel;
    juce::Label outputLabel;

    // ==========================================================
    // TUNER
    // ==========================================================

    juce::Label tunerTitleLabel;
    juce::Label tunerStatusLabel;

    juce::Label detectedNoteLabel;
    juce::Label detectedFrequencyLabel;
    juce::Label detectedCentsLabel;
    juce::Label detectedConfidenceLabel;

    juce::ComboBox keyComboBox;
    juce::ComboBox scaleComboBox;
    juce::ComboBox modeComboBox;

    // juce::Slider tunerRetuneSlider;
    // juce::Slider tunerSmoothSlider;
    // juce::Slider tunerFormantSlider;
    // juce::Slider tunerMixSlider;

    ProfessionalKnob tunerRetuneSlider;
    ProfessionalKnob tunerSmoothSlider;
    ProfessionalKnob tunerFormantSlider;
    ProfessionalKnob tunerMixSlider;

    juce::Label keyLabel;
    juce::Label scaleLabel;
    juce::Label modeLabel;

    juce::Label tunerRetuneLabel;
    juce::Label tunerSmoothLabel;
    juce::Label tunerFormantLabel;
    juce::Label tunerMixLabel;

    // ==========================================================
    // DOUBLER
    // ==========================================================

    juce::Label doublerTitleLabel;

    // juce::Slider doublerAmountSlider;
    // juce::Slider doublerDetuneSlider;
    // juce::Slider doublerTimingSlider;
    // juce::Slider doublerWidthSlider;
    // juce::Slider doublerMixSlider;

    ProfessionalKnob doublerAmountSlider;
    ProfessionalKnob doublerDetuneSlider;
    ProfessionalKnob doublerTimingSlider;
    ProfessionalKnob doublerWidthSlider;
    ProfessionalKnob doublerMixSlider;

    juce::Label doublerAmountLabel;
    juce::Label doublerDetuneLabel;
    juce::Label doublerTimingLabel;
    juce::Label doublerWidthLabel;
    juce::Label doublerMixLabel;

    // ==========================================================
    // HARMONY
    // ==========================================================

    juce::Label harmonyTitleLabel;

    juce::ComboBox harmonyVoice1ComboBox;
    juce::ComboBox harmonyVoice2ComboBox;
    juce::ComboBox harmonyVoice3ComboBox;
    juce::ComboBox harmonyVoice4ComboBox;

    juce::Label harmonyVoice1Label;
    juce::Label harmonyVoice2Label;
    juce::Label harmonyVoice3Label;
    juce::Label harmonyVoice4Label;

    // juce::Slider harmonyMixSlider;
    ProfessionalKnob harmonyMixSlider;
    juce::Label harmonyMixLabel;

    // ==========================================================
    // CREATIVE FX
    // ==========================================================

    juce::Label creativeFxTitleLabel;

    juce::ComboBox creativeFxTypeComboBox;

    // juce::Slider creativeFxAmountSlider;
    // juce::Slider creativeFxMixSlider;
    ProfessionalKnob creativeFxAmountSlider;
    ProfessionalKnob creativeFxMixSlider;

    juce::Label creativeFxTypeLabel;
    juce::Label creativeFxAmountLabel;
    juce::Label creativeFxMixLabel;

    // ==========================================================
    // SPACE
    // ==========================================================

    juce::Label spaceTitleLabel;

    juce::ComboBox spaceTypeComboBox;

    // juce::Slider spaceSizeSlider;
    // juce::Slider spaceDecaySlider;
    // juce::Slider spacePreDelaySlider;
    // juce::Slider spaceDampingSlider;
    // juce::Slider spaceMixSlider;
    ProfessionalKnob spaceSizeSlider;
    ProfessionalKnob spaceDecaySlider;
    ProfessionalKnob spacePreDelaySlider;
    ProfessionalKnob spaceDampingSlider;
    ProfessionalKnob spaceMixSlider;

    juce::Label spaceTypeLabel;
    juce::Label spaceSizeLabel;
    juce::Label spaceDecayLabel;
    juce::Label spacePreDelayLabel;
    juce::Label spaceDampingLabel;
    juce::Label spaceMixLabel;

    // ==========================================================
    // APVTS ATTACHMENTS
    // ==========================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        inputAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        globalMixAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        outputAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ButtonAttachment>
        bypassAttachment;

    // ==========================================================
    // TUNER ATTACHMENTS
    // ==========================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        tunerKeyAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        tunerScaleAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        tunerModeAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        tunerRetuneAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        tunerSmoothAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        tunerFormantAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        tunerMixAttachment;

    // ==========================================================
    // DOUBLER ATTACHMENTS
    // ==========================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        doublerAmountAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        doublerDetuneAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        doublerTimingAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        doublerWidthAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        doublerMixAttachment;

    // ==========================================================
    // HARMONY ATTACHMENTS
    // ==========================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        harmonyVoice1Attachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        harmonyVoice2Attachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        harmonyVoice3Attachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        harmonyVoice4Attachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        harmonyMixAttachment;

    // ==========================================================
    // CREATIVE FX ATTACHMENTS
    // ==========================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        creativeFxTypeAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        creativeFxAmountAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        creativeFxMixAttachment;

    // ==========================================================
    // SPACE ATTACHMENTS
    // ==========================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        spaceTypeAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        spaceSizeAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        spaceDecayAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        spacePreDelayAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        spaceDampingAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        spaceMixAttachment;

    // ==========================================================
    // UI HELPERS
    // ==========================================================

    void setupSlider(
        juce::Slider& slider);

    void setupGlobalSlider(
        juce::Slider& slider);

    void setupLabel(
        juce::Label& label,
        const juce::String& text);

    void setupTitle(
        juce::Label& label,
        const juce::String& text);

    void setupComboBox(
        juce::ComboBox& comboBox);

    void setupPitchDisplayLabel(
        juce::Label& label);

    void setupModuleButton(
        ModuleButton& button,
        const juce::Image& icon,
        const juce::String& name);

    void drawPanel(
        juce::Graphics& g,
        juce::Rectangle<int> bounds,
        bool highlighted = false);

    void drawPitchMeter(
        juce::Graphics& g,
        juce::Rectangle<int> bounds);

    void drawModuleHeader(
        juce::Graphics& g,
        juce::Rectangle<int> bounds,
        const juce::String& title);

    void drawGlobalSection(
        juce::Graphics& g,
        juce::Rectangle<int> bounds);

    void loadImages();

    // ==========================================================
    // COMBO DATA
    // ==========================================================

    void populateKeyComboBox();

    void populateScaleComboBox();

    void populateModeComboBox();

    void populateHarmonyComboBoxes();

    void populateCreativeFxComboBox();

    void populateSpaceComboBox();

    // ==========================================================
    // TIMER
    // ==========================================================

    void timerCallback() override;

    // ==========================================================
    // PROCESSOR
    // ==========================================================

    OfforVocalProAudioProcessor& audioProcessor;

    // ==========================================================
    // COLOURS
    // ==========================================================

    static const juce::Colour backgroundColour;
    static const juce::Colour panelColour;
    static const juce::Colour panelColour2;
    static const juce::Colour borderColour;
    static const juce::Colour textColour;
    static const juce::Colour mutedColour;
    static const juce::Colour accentColour;
    static const juce::Colour accentDarkColour;
    static const juce::Colour displayColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        OfforVocalProAudioProcessorEditor
    )
};