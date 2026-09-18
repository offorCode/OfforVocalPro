#pragma once

#include <JuceHeader.h>

//==============================================================================
// OFFOR VOCAL PRO - SETTINGS PANEL
//
// This component is intentionally kept separate from PluginEditor.
//
// PluginEditor should only:
//      1. Create SettingsPanel
//      2. Show it
//      3. Hide it
//
// All settings UI, navigation and page drawing lives here.
//
//==============================================================================

class SettingsPanel : public juce::Component
{
public:

    //==========================================================================
    // Callback used by PluginEditor to close the settings screen.
    //
    // Example:
    //
    // settingsPanel->onClose = [this]()
    // {
    //     settingsPanel->setVisible(false);
    // };
    //
    std::function<void()> onClose;

    //==========================================================================
    SettingsPanel();
    ~SettingsPanel() override = default;

    //==========================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:

    //==========================================================================
    // SETTINGS PAGES
    //==========================================================================

    enum class Page
    {
        general,
        audio,
        display,
        performance,
        about
    };

    Page currentPage = Page::general;

    //==========================================================================
    // CUSTOM TAB BUTTON
    //==========================================================================

    class TabButton : public juce::Button
    {
    public:

        explicit TabButton(const juce::String& text);

        void setSelected(bool shouldBeSelected);

        void paintButton(
            juce::Graphics& g,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;

    private:

        juce::String buttonText;
        bool selected = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabButton)
    };

    TabButton generalTab      { "GENERAL" };
    TabButton audioTab        { "AUDIO" };
    TabButton displayTab      { "DISPLAY" };
    TabButton performanceTab  { "PERFORMANCE" };
    TabButton aboutTab        { "ABOUT" };

    //==========================================================================
    // CLOSE BUTTON
    //==========================================================================

    class CloseButton : public juce::Button
    {
    public:

        CloseButton();

        void paintButton(
            juce::Graphics& g,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;
    };

    CloseButton closeButton;

    //==========================================================================
    // CUSTOM TOGGLE
    //==========================================================================

    class ToggleSwitch : public juce::Button
    {
    public:

        ToggleSwitch();

        void setToggleState(
            bool shouldBeOn,
            juce::NotificationType notification);

        bool getToggleState() const;

        void paintButton(
            juce::Graphics& g,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;

    private:

        bool isOn = true;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitch)
    };

    //==========================================================================
    // CUSTOM SELECTOR
    //==========================================================================

    class SettingSelector : public juce::Component
    {
    public:

        explicit SettingSelector(
            const juce::String& initialValue = "Default");

        void setValue(const juce::String& newValue);
        juce::String getValue() const;

        void paint(juce::Graphics& g) override;
        void resized() override;

        std::function<void()> onClicked;

    private:

        juce::String value;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingSelector)
    };

    //==========================================================================
    // GENERAL PAGE
    //==========================================================================

    juce::Label generalTitle;
    juce::Label generalDescription;

    juce::Label pluginBehaviourLabel;
    juce::Label enableProcessingLabel;

    ToggleSwitch enableProcessingToggle;

    juce::Label interfaceLabel;
    juce::Label uiScaleLabel;
    juce::Label themeLabel;

    SettingSelector uiScaleSelector;
    SettingSelector themeSelector;

    //==========================================================================
    // AUDIO PAGE
    //==========================================================================

    juce::Label audioTitle;
    juce::Label audioDescription;

    juce::Label inputBehaviourLabel;
    juce::Label inputGainLabel;
    juce::Label processingLabel;
    juce::Label oversamplingLabel;
    juce::Label qualityLabel;

    SettingSelector oversamplingSelector;
    SettingSelector qualitySelector;

    //==========================================================================
    // DISPLAY PAGE
    //==========================================================================

    juce::Label displayTitle;
    juce::Label displayDescription;

    juce::Label appearanceLabel;
    juce::Label displayThemeLabel;
    juce::Label displayScaleLabel;

    juce::Label visualLabel;
    juce::Label inputMeterLabel;
    juce::Label outputMeterLabel;
    juce::Label tooltipsLabel;

    ToggleSwitch inputMeterToggle;
    ToggleSwitch outputMeterToggle;
    ToggleSwitch tooltipsToggle;

    SettingSelector displayThemeSelector;
    SettingSelector displayScaleSelector;

    //==========================================================================
    // PERFORMANCE PAGE
    //==========================================================================

    juce::Label performanceTitle;
    juce::Label performanceDescription;

    juce::Label engineLabel;
    juce::Label cpuLabel;
    juce::Label processingQualityLabel;

    SettingSelector cpuModeSelector;
    SettingSelector processingQualitySelector;

    //==========================================================================
    // ABOUT PAGE
    //==========================================================================

    juce::Label aboutTitle;
    juce::Label aboutDescription;

    juce::Label productNameLabel;
    juce::Label versionLabel;
    juce::Label companyLabel;
    juce::Label descriptionLabel;

    juce::Label licenseTitleLabel;
    juce::Label licenseStatusLabel;

    juce::TextButton websiteButton;
    juce::TextButton supportButton;

    //==========================================================================
    // PAGE MANAGEMENT
    //==========================================================================

    void selectPage(Page page);
    void updatePageVisibility();

    void setupLabels();
    void setupTabs();
    void setupControls();
    void setupAboutPage();

    void configureLabel(
        juce::Label& label,
        const juce::String& text,
        float fontSize,
        juce::Colour colour,
        juce::Justification justification =
            juce::Justification::centredLeft);

    //==========================================================================
    // PAGE DRAWING
    //==========================================================================

    void drawPageHeader(
        juce::Graphics& g,
        const juce::String& title,
        const juce::String& description);

    void drawSectionLine(
        juce::Graphics& g,
        int y);

    void drawSettingRow(
        juce::Graphics& g,
        const juce::String& title,
        const juce::String& description,
        int y,
        int width);

    //==========================================================================
    // COLOURS
    //==========================================================================

    static const juce::Colour backgroundColour;
    static const juce::Colour panelColour;
    static const juce::Colour panelColour2;
    static const juce::Colour borderColour;
    static const juce::Colour textColour;
    static const juce::Colour mutedColour;
    static const juce::Colour accentColour;
    static const juce::Colour accentDarkColour;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
};