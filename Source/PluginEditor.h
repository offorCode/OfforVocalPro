#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LevelMeter.h"
#include "UI/SettingsPanel.h"
#include "UI/ThemeManager.h"
#include "UI/ThemeColorPanel.h"

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
    // THEME SYSTEM
    // ==========================================================

    void applyTheme();

    void refreshThemeColours();

    ThemeManager::Colours getThemeColours() const;

    void updateThemeForChildComponents();

    // ==========================================================
    // CUSTOM THEME COLOUR PANEL
    // ==========================================================
    //
    // This panel is created only when the user chooses
    // "Custom" / "Customize Colours" from Settings.
    //
    // ==========================================================

    std::unique_ptr<ThemeColorPanel> themeColorPanel;

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

    // ==========================================================
    // SETTINGS BUTTON
    // ==========================================================
    //
    // The SETTINGS text is replaced by setting.png.
    //
    // The image will be loaded from BinaryData and recoloured
    // white in PluginEditor.cpp.
    //

    // SETTINGS icon button.
    // Uses the white settings.png image directly.
    juce::ImageButton settingsButton;

    juce::Image settingsIcon;

    // SETTINGS PANEL
    // Separate professional settings screen shown over the main plugin UI.
    std::unique_ptr<SettingsPanel> settingsPanel;

    void showSettingsPanel();
    void hideSettingsPanel();

        //==========================================================================
    // SETTINGS CALLBACKS
    //==========================================================================
    //
    // These functions receive events from SettingsPanel.
    //
    // SettingsPanel does not directly control PluginProcessor.
    //
    // Flow:
    //
    // SettingsPanel
    //      ↓
    // PluginEditor
    //      ↓
    // Processor / UI
    //
    //==========================================================================

    void setupSettingsCallbacks();

    void handleProcessingChanged(
        bool enabled);

    void handleUIScaleChanged(
        const juce::String& scale);

    void handleThemeChanged(
        const juce::String& theme);

    void handleOversamplingChanged(
        const juce::String& mode);

    void handleProcessingQualityChanged(
        const juce::String& quality);

    void handleInputMeterChanged(
        bool visible);

    void handleOutputMeterChanged(
        bool visible);

    void handleTooltipsChanged(
        bool enabled);

    void handleDisplayScaleChanged(
        const juce::String& scale);

    void handleCPUModeChanged(
        const juce::String& mode);

    //==========================================================================
    // SETTINGS UI STATE
    //==========================================================================

    bool tooltipsEnabled = true;

    // ==========================================================
    // THEME
    // ==========================================================
    //
    // ThemeManager is the single source of truth for all colours.
    //
    // Built-in themes:
    //
    // OFFOR Dark
    // Midnight Blue
    // Graphite
    // Purple Studio
    // Emerald
    // Crimson
    // Light
    // Custom
    //
    // ==========================================================

    juce::String currentTheme = "OFFOR Dark";

    juce::String currentUIScale = "100%";

    juce::String currentDisplayScale = "100%";

    double uiScale = 1.0;

    // Base/reference editor dimensions at 100%.
    static constexpr int baseEditorWidth  = 920;
    static constexpr int baseEditorHeight = 570;

    // Convert SettingsPanel percentage text into a scale factor.
    double getUIScaleFactor(
        const juce::String& scale) const;

    // Apply the selected scale to the editor.
    void applyUIScale(
        const juce::String& scale);

    // ==========================================================
    // SCALED BOUNDS HELPERS
    // ==========================================================
    //
    // These convert 100%-design coordinates into actual pixels.
    //
    // Example:
    //
    //     setScaledBounds(component, 68, 11, 240, 24);
    //
    // At 100%:
    //
    //     68, 11, 240, 24
    //
    // At 125%:
    //
    //     85, 14, 300, 30
    //
    // ==========================================================

    int scaleValue(
        int value) const;

    juce::Rectangle<int> scaledBounds(
        int x,
        int y,
        int width,
        int height) const;

    void setScaledBounds(
        juce::Component& component,
        int x,
        int y,
        int width,
        int height) const;

    juce::String currentOversampling = "2X";

    juce::String currentProcessingQuality = "High";

    juce::String currentCPUMode = "Balanced";

    juce::ToggleButton bypassButton;

    juce::Image radioTunerIcon;
    juce::Image doublerIcon;
    juce::Image harmonyIcon;
    juce::Image fxIcon;
    juce::Image spaceIcon;

    // ==========================================================
    // PRESET SYSTEM
    // ==========================================================
    //
    // Presets are shown directly in the header.
    //
    // SAVE opens a file chooser and stores the complete APVTS
    // state as an OFFOR VOCAL PRO preset file.
    //

    juce::ComboBox presetComboBox;
    // ==========================================================
    // PRESET MENU BUTTON
    // ==========================================================
    //
    // LOAD and SAVE are intentionally hidden inside this menu.
    // This keeps the header compact and professional.
    //

    juce::TextButton presetMenuButton;

    // Keeps the asynchronous JUCE FileChooser alive while
    // the user is selecting a save location.
    std::unique_ptr<juce::FileChooser> presetFileChooser;

    // ==========================================================
    // A / B COMPARISON
    // ==========================================================
    //
    // A and B each contain a complete copy of the APVTS state.
    //
    // This allows the entire plugin configuration to be compared,
    // rather than only comparing individual parameters.
    //

    juce::TextButton aButton;
    juce::TextButton bButton;

    juce::ValueTree stateA;
    juce::ValueTree stateB;

    bool isAActive = true;

        // ==========================================================
    // LICENSE OVERLAY
    // ==========================================================
    //
    // IMPORTANT:
    //
    // The editor MUST remain open after the free trial is
    // exhausted. The user needs access to this screen in order
    // to enter their license key.
    //
    // When the trial is exhausted:
    //
    //     UI remains visible
    //     DSP is blocked by PluginProcessor
    //     License overlay becomes visible
    //
    // After successful activation:
    //
    //     overlay disappears
    //     DSP becomes available again
    //
    // ==========================================================

    class LicenseActionButton
        : public juce::Button
    {
    public:

        explicit LicenseActionButton(
            const juce::String& buttonText);

        void paintButton(
            juce::Graphics& g,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;

    private:

        juce::String text;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            LicenseActionButton
        )
    };


    class LicenseOverlay
        : public juce::Component
    {
    public:

        explicit LicenseOverlay(
            OfforVocalProAudioProcessorEditor& owner);

        ~LicenseOverlay() override;

        void paint(
            juce::Graphics& g) override;

        void resized() override;

        
        void setActivationState(
            bool activating,
            const juce::String& message = {});

        void clearActivationMessage();

        juce::String getLicenseKey() const;

        void setLicenseKey(
            const juce::String& key);

        LicenseActionButton activateButton;
        LicenseActionButton buyButton;

        juce::TextEditor licenseEditor;

    private:

        OfforVocalProAudioProcessorEditor& owner;

        juce::Label titleLabel;
        juce::Label messageLabel;
        juce::Label instructionLabel;
        juce::Label statusLabel;

        bool activating = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            LicenseOverlay
        )
    };


    // The overlay is owned by the editor.
    //
    // It sits above the normal plugin interface and is only
    // displayed when the trial has been exhausted.
    std::unique_ptr<LicenseOverlay> licenseOverlay;


    // ==========================================================
    // LICENSE ACTIVATION THREAD
    // ==========================================================

    class LicenseActivationThread : public juce::Thread
    {
    public:

        LicenseActivationThread(
            OfforVocalProAudioProcessorEditor& editor,
            const juce::String& key);

        void run() override;

    private:

        OfforVocalProAudioProcessorEditor& owner;
        juce::String licenseKey;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            LicenseActivationThread
        )
    };

    std::unique_ptr<LicenseActivationThread> licenseActivationThread;


    // ==========================================================
    // LICENSE HELPERS
    // ==========================================================

    void setupLicenseOverlay();

    void updateLicenseOverlay();

    void beginLicenseActivation();

    void finishLicenseActivation(
        bool success,
        const juce::String& message);

    void openLicensePurchasePage();

    // ==========================================================
    // PRESET / A-B HELPERS
    // ==========================================================

    void setupPresetControls();

    void loadFactoryPreset(int presetIndex);

    void saveCurrentStateToA();

    void saveCurrentStateToB();

    void recallStateA();

    void recallStateB();

    void updateABButtonStates();

    // ==========================================================
    // FILE PRESET HELPERS
    // ==========================================================

    // Save the complete APVTS state to a .offorvocalpreset file.
    void savePresetToFile();

    // Load a previously saved .offorvocalpreset file.
    void loadPresetFromFile(
        const juce::File& file);

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

    ProfessionalKnob inputSlider;
    ProfessionalKnob globalMixSlider;
    ProfessionalKnob outputSlider;

    juce::Label inputLabel;
    juce::Label globalMixLabel;
    juce::Label outputLabel;

    // ==========================================================
    // LEVEL METERS
    // ==========================================================
    //
    // Visualization-only meters.
    //
    // They do NOT replace the INPUT or OUTPUT knobs.
    //

    LevelMeter inputMeter;
    LevelMeter outputMeter;

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

    ProfessionalKnob harmonyMixSlider;
    juce::Label harmonyMixLabel;

    // ==========================================================
    // CREATIVE FX
    // ==========================================================

    juce::Label creativeFxTitleLabel;

    juce::ComboBox creativeFxTypeComboBox;

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

    // FIX:
    // AudioProcessorValueTreeState is the correct APVTS namespace.
    //
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