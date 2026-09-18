#pragma once

#include <JuceHeader.h>

#include "ThemeManager.h"
#include "ThemeColorPanel.h"

#include "FeedbackPanel.h"

//==============================================================================
// OFFOR VOCAL PRO - SETTINGS PANEL
//
// SettingsPanel is responsible for:
//
//      1. Settings UI
//      2. Settings navigation
//      3. Dropdowns
//      4. Toggles
//      5. Theme selection
//      6. Custom theme colour editor
//      7. Built-in User Guide
//
// PluginEditor remains responsible for connecting the settings
// callbacks to the actual processor/UI behaviour.
//
//==============================================================================

class SettingsPanel
    : public juce::Component,
      private juce::ChangeListener,
      private juce::Timer
{
public:

    //==========================================================================
    // CALLBACKS
    //==========================================================================

    std::function<void()> onClose;

    std::function<void(bool)> onProcessingChanged;

    std::function<void(const juce::String&)> onUIScaleChanged;
    std::function<void(const juce::String&)> onThemeChanged;
    std::function<void(const juce::String&)> onDisplayThemeChanged;

    std::function<void(const juce::String&)> onOversamplingChanged;
    std::function<void(const juce::String&)> onProcessingQualityChanged;

    std::function<void(bool)> onInputMeterChanged;
    std::function<void(bool)> onOutputMeterChanged;
    std::function<void(bool)> onTooltipsChanged;
    std::function<void(const juce::String&)> onDisplayScaleChanged;

    std::function<void(const juce::String&)> onCPUModeChanged;

    //==========================================================================
    SettingsPanel();

    ~SettingsPanel() override;

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
        about,
        userGuide,
        feedback
    };

    Page currentPage = Page::general;

    //==========================================================================
    // TAB BUTTON
    //==========================================================================

    class TabButton : public juce::Button
    {
    public:

        explicit TabButton(
            const juce::String& text);

        void setSelected(
            bool shouldBeSelected);

        void paintButton(
            juce::Graphics& g,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override;

    private:

        juce::String buttonText;
        bool selected = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabButton)
    };

    TabButton generalTab     { "GENERAL" };
    TabButton audioTab       { "AUDIO" };
    TabButton displayTab     { "DISPLAY" };
    TabButton performanceTab { "PERFORMANCE" };
    TabButton aboutTab       { "ABOUT" };
    TabButton userGuideTab   { "USER GUIDE" };
    TabButton feedbackTab { "FEEDBACK" };

    //==========================================================================
    // FEEDBACK
    //==========================================================================
    //
    // The complete feedback system lives inside FeedbackPanel.
    //
    // SettingsPanel only controls which page is visible.
    //
    //==========================================================================

    std::unique_ptr<FeedbackPanel> feedbackPanel;

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

        void setValue(
            const juce::String& newValue);

        juce::String getValue() const;

        void setOptions(
            const juce::StringArray& newOptions);

        void showMenu();

        void paint(
            juce::Graphics& g) override;

        void resized() override;

        void mouseEnter(
            const juce::MouseEvent& event) override;

        void mouseExit(
            const juce::MouseEvent& event) override;

        void mouseDown(
            const juce::MouseEvent& event) override;

        std::function<void()> onClicked;

    private:

        juce::String value;

        juce::StringArray options;

        bool mouseOver = false;

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
    // CUSTOM THEME BUTTON
    //==========================================================================

    juce::TextButton customizeColoursButton;

    std::unique_ptr<ThemeColorPanel> themeColorPanel;

    bool customColoursVisible = false;

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
    // USER GUIDE
    //==========================================================================
    //
    // The guide is intentionally implemented as a Viewport + Label.
    //
    // This keeps the system lightweight and avoids introducing another
    // complex UI component into the plugin.
    //
    // The guide automatically scrolls using the SettingsPanel timer.
    //
    //==========================================================================
    
    juce::Label userGuideTitle;
    juce::Label userGuideDescription;

    juce::Viewport userGuideViewport;
    juce::Label userGuideContent;

    // Current automatic scroll position.
    float userGuideScrollPosition = 0.0f;

    // True while the user is interacting with the guide.
    //
    // Automatic scrolling pauses while the mouse is over the guide
    // or while the user is manually scrolling.
    bool userGuideMouseOver = false;

    // Used to give the guide a small delay before auto-scroll resumes.
    int userGuideIdleCounter = 0;

    //==========================================================================
    // PAGE MANAGEMENT
    //==========================================================================

    void selectPage(Page page);
    void updatePageVisibility();

    void setupLabels();
    void setupTabs();
    void setupControls();
    void setupAboutPage();

    //==========================================================================
    // USER GUIDE
    //==========================================================================

    void setupUserGuide();

    juce::String getUserGuideText() const;

    void resetUserGuideScroll();

    void updateUserGuideLayout();

    //==========================================================================
    // TIMER
    //==========================================================================

    void timerCallback() override;

    //==========================================================================
    // SETTINGS CALLBACKS
    //==========================================================================

    void setupSettingCallbacks();

    //==========================================================================
    // THEME
    //==========================================================================

    void setupThemeControls();

    void showThemeColorPanel();

    void hideThemeColorPanel();

    void updateThemeColours();

    void syncThemeSelectors(
        const juce::String& theme);

    //==========================================================================
    // THEME CHANGE LISTENER
    //==========================================================================

    void changeListenerCallback(
        juce::ChangeBroadcaster* source) override;

    //==========================================================================
    // PROCESSING QUALITY
    //==========================================================================

    void syncProcessingQualitySelectors(
        const juce::String& quality);

    //==========================================================================
    // LABEL HELPER
    //==========================================================================

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
    // LEGACY STATIC COLOURS
    //==========================================================================
    //
    // Kept so the rest of the existing SettingsPanel code does not break.
    //
    // New painting code uses ThemeManager directly.
    //
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