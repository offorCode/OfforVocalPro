#include "SettingsPanel.h"

//==============================================================================
// COLOURS
//==============================================================================

const juce::Colour SettingsPanel::backgroundColour =
    juce::Colour(0xff111216);

const juce::Colour SettingsPanel::panelColour =
    juce::Colour(0xff17181d);

const juce::Colour SettingsPanel::panelColour2 =
    juce::Colour(0xff1d1f25);

const juce::Colour SettingsPanel::borderColour =
    juce::Colour(0xff30323a);

const juce::Colour SettingsPanel::textColour =
    juce::Colour(0xfff2f2f4);

const juce::Colour SettingsPanel::mutedColour =
    juce::Colour(0xff858791);

const juce::Colour SettingsPanel::accentColour =
    juce::Colour(0xffdf513e);

const juce::Colour SettingsPanel::accentDarkColour =
    juce::Colour(0xffa93629);

//==============================================================================
// TAB BUTTON
//==============================================================================

SettingsPanel::TabButton::TabButton(const juce::String& text)
    : juce::Button(text),
      buttonText(text)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SettingsPanel::TabButton::setSelected(bool shouldBeSelected)
{
    selected = shouldBeSelected;
    repaint();
}

void SettingsPanel::TabButton::paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsDown);

    auto bounds = getLocalBounds().toFloat();

    //--------------------------------------------------------------------------
    // Hover background
    //--------------------------------------------------------------------------

    if (shouldDrawButtonAsHighlighted && ! selected)
    {
        g.setColour(
            SettingsPanel::panelColour2.withAlpha(0.55f));

        g.fillRoundedRectangle(
            bounds.reduced(2.0f),
            6.0f);
    }

    //--------------------------------------------------------------------------
    // Text
    //--------------------------------------------------------------------------

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)
                .withStyle("Bold")));

    g.setColour(
        selected
            ? SettingsPanel::textColour
            : SettingsPanel::mutedColour);

    g.drawText(
        buttonText,
        getLocalBounds().reduced(4, 0),
        juce::Justification::centred,
        false);

    //--------------------------------------------------------------------------
    // Active underline
    //--------------------------------------------------------------------------

    if (selected)
    {
        g.setColour(SettingsPanel::accentColour);

        auto underline =
            bounds.withHeight(2.5f)
                  .withY(bounds.getBottom() - 2.5f)
                  .reduced(18.0f, 0.0f);

        g.fillRoundedRectangle(
            underline,
            1.25f);
    }
}

//==============================================================================
// CLOSE BUTTON
//==============================================================================

SettingsPanel::CloseButton::CloseButton()
    : juce::Button("Close")
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SettingsPanel::CloseButton::paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsDown);

    auto bounds = getLocalBounds().toFloat();

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(
            SettingsPanel::panelColour2);

        g.fillRoundedRectangle(
            bounds.reduced(1.0f),
            7.0f);
    }

    g.setColour(
        shouldDrawButtonAsHighlighted
            ? SettingsPanel::textColour
            : SettingsPanel::mutedColour);

    g.drawLine(
        bounds.getCentreX() - 5.0f,
        bounds.getCentreY() - 5.0f,
        bounds.getCentreX() + 5.0f,
        bounds.getCentreY() + 5.0f,
        1.7f);

    g.drawLine(
        bounds.getCentreX() + 5.0f,
        bounds.getCentreY() - 5.0f,
        bounds.getCentreX() - 5.0f,
        bounds.getCentreY() + 5.0f,
        1.7f);
}

//==============================================================================
// TOGGLE SWITCH
//==============================================================================

SettingsPanel::ToggleSwitch::ToggleSwitch()
    : juce::Button("Toggle")
{
    setClickingTogglesState(true);

    setToggleState(
        true,
        juce::dontSendNotification);

    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);
}

void SettingsPanel::ToggleSwitch::setToggleState(
    bool shouldBeOn,
    juce::NotificationType notification)
{
    isOn = shouldBeOn;

    juce::Button::setToggleState(
        shouldBeOn,
        notification);

    repaint();
}

bool SettingsPanel::ToggleSwitch::getToggleState() const
{
    return isOn;
}

void SettingsPanel::ToggleSwitch::paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(
        shouldDrawButtonAsDown);

    auto bounds = getLocalBounds().toFloat();

    const float width = bounds.getWidth();
    const float height = 24.0f;

    auto track =
        juce::Rectangle<float>(
            0.0f,
            (bounds.getHeight() - height) * 0.5f,
            width,
            height);

    //--------------------------------------------------------------------------
    // Track
    //--------------------------------------------------------------------------

    g.setColour(
        isOn
            ? SettingsPanel::accentColour
            : SettingsPanel::borderColour);

    g.fillRoundedRectangle(
        track,
        height * 0.5f);

    //--------------------------------------------------------------------------
    // Track highlight
    //--------------------------------------------------------------------------

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(
            juce::Colours::white.withAlpha(0.08f));

        g.fillRoundedRectangle(
            track,
            height * 0.5f);
    }

    //--------------------------------------------------------------------------
    // Knob
    //--------------------------------------------------------------------------

    const float knobSize = height - 6.0f;

    const float knobX =
        isOn
            ? track.getRight() - knobSize - 3.0f
            : track.getX() + 3.0f;

    auto knob =
        juce::Rectangle<float>(
            knobX,
            track.getY() + 3.0f,
            knobSize,
            knobSize);

    g.setColour(
        juce::Colours::white);

    g.fillEllipse(knob);

    //--------------------------------------------------------------------------
    // Small ON/OFF text
    //--------------------------------------------------------------------------

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(8.0f)
                .withStyle("Bold")));

    g.setColour(
        isOn
            ? juce::Colours::white
            : SettingsPanel::mutedColour);

    g.drawText(
        isOn ? "ON" : "OFF",
        track.toNearestInt(),
        juce::Justification::centred,
        false);
}

//==============================================================================
// SETTING SELECTOR
//==============================================================================

SettingsPanel::SettingSelector::SettingSelector(
    const juce::String& initialValue)
    : value(initialValue)
{
    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    setRepaintsOnMouseActivity(true);

    //--------------------------------------------------------------------------
    // Simple click behaviour for now.
    //
    // Later these selectors can become proper popup menus or ComboBoxes
    // connected to APVTS parameters.
    //--------------------------------------------------------------------------

    setInterceptsMouseClicks(true, true);
}


void SettingsPanel::SettingSelector::setValue(
    const juce::String& newValue)
{
    value = newValue;
    repaint();
}

juce::String SettingsPanel::SettingSelector::getValue() const
{
    return value;
}

void SettingsPanel::SettingSelector::paint(
    juce::Graphics& g)
{
    auto bounds =
        getLocalBounds().toFloat();

    const bool highlighted =
        isMouseOver();

    //--------------------------------------------------------------------------
    // Background
    //--------------------------------------------------------------------------

    g.setColour(
        highlighted
            ? SettingsPanel::panelColour2
            : SettingsPanel::panelColour);

    g.fillRoundedRectangle(
        bounds,
        7.0f);

    //--------------------------------------------------------------------------
    // Border
    //--------------------------------------------------------------------------

    g.setColour(
        highlighted
            ? SettingsPanel::accentDarkColour
            : SettingsPanel::borderColour);

    g.drawRoundedRectangle(
        bounds.reduced(0.5f),
        7.0f,
        1.0f);

    //--------------------------------------------------------------------------
    // Value
    //--------------------------------------------------------------------------

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)
                .withStyle("Medium")));

    g.setColour(
        SettingsPanel::textColour);

    g.drawText(
        value,
        bounds.reduced(12.0f, 0.0f)
             .withRightX(bounds.getRight() - 25.0f),
        juce::Justification::centredLeft,
        false);

    //--------------------------------------------------------------------------
    // Arrow
    //--------------------------------------------------------------------------

    const float arrowX =
        bounds.getRight() - 17.0f;

    const float arrowY =
        bounds.getCentreY();

    juce::Path arrow;

    arrow.startNewSubPath(
        arrowX - 4.0f,
        arrowY - 2.0f);

    arrow.lineTo(
        arrowX,
        arrowY + 2.0f);

    arrow.lineTo(
        arrowX + 4.0f,
        arrowY - 2.0f);

    g.setColour(
        SettingsPanel::mutedColour);

    g.strokePath(
        arrow,
        juce::PathStrokeType(
            1.4f,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));
}

// ==========================================================
// SETTING SELECTOR - RESIZED
// ==========================================================

void SettingsPanel::SettingSelector::resized()
{
    // The selector is currently a custom visual control.
    // There are no child components inside it yet.
    //
    // This function is intentionally empty for now.
    // It exists because SettingSelector inherits from
    // juce::Component and declares resized() in the header.
}


//==============================================================================
// SETTINGS PANEL
//==============================================================================

SettingsPanel::SettingsPanel()
{
    //==========================================================================
    // Basic component configuration
    //==========================================================================

    setOpaque(true);

    //==========================================================================
    // Tabs
    //==========================================================================

    setupTabs();

    //==========================================================================
    // Labels
    //==========================================================================

    setupLabels();

    //==========================================================================
    // Controls
    //==========================================================================

    setupControls();

    //==========================================================================
    // About
    //==========================================================================

    setupAboutPage();

    //==========================================================================
    // Initial page
    //==========================================================================

    selectPage(Page::general);
}

//==============================================================================
// SETUP TABS
//==============================================================================

void SettingsPanel::setupTabs()
{
    addAndMakeVisible(generalTab);
    addAndMakeVisible(audioTab);
    addAndMakeVisible(displayTab);
    addAndMakeVisible(performanceTab);
    addAndMakeVisible(aboutTab);

    generalTab.onClick =
        [this]()
        {
            selectPage(Page::general);
        };

    audioTab.onClick =
        [this]()
        {
            selectPage(Page::audio);
        };

    displayTab.onClick =
        [this]()
        {
            selectPage(Page::display);
        };

    performanceTab.onClick =
        [this]()
        {
            selectPage(Page::performance);
        };

    aboutTab.onClick =
        [this]()
        {
            selectPage(Page::about);
        };

    //==========================================================================
    // Close
    //==========================================================================

    addAndMakeVisible(closeButton);

    closeButton.onClick =
        [this]()
        {
            if (onClose)
                onClose();
        };
}

//==============================================================================
// SETUP LABELS
//==============================================================================

void SettingsPanel::setupLabels()
{
    //==========================================================================
    // GENERAL
    //==========================================================================

    configureLabel(
        generalTitle,
        "GENERAL",
        20.0f,
        textColour);

    configureLabel(
        generalDescription,
        "Configure the basic behaviour and appearance of Offor Vocal Pro.",
        11.0f,
        mutedColour);

    configureLabel(
        pluginBehaviourLabel,
        "PLUGIN BEHAVIOUR",
        11.0f,
        accentColour);

    configureLabel(
        enableProcessingLabel,
        "Enable Processing",
        12.0f,
        textColour);

    configureLabel(
        interfaceLabel,
        "INTERFACE",
        11.0f,
        accentColour);

    configureLabel(
        uiScaleLabel,
        "UI Scale",
        12.0f,
        textColour);

    configureLabel(
        themeLabel,
        "Theme",
        12.0f,
        textColour);

    //==========================================================================
    // AUDIO
    //==========================================================================

    configureLabel(
        audioTitle,
        "AUDIO",
        20.0f,
        textColour);

    configureLabel(
        audioDescription,
        "Configure audio quality and processing behaviour.",
        11.0f,
        mutedColour);

    configureLabel(
        inputBehaviourLabel,
        "INPUT BEHAVIOUR",
        11.0f,
        accentColour);

    configureLabel(
        inputGainLabel,
        "Input Gain",
        12.0f,
        textColour);

    configureLabel(
        processingLabel,
        "PROCESSING",
        11.0f,
        accentColour);

    configureLabel(
        oversamplingLabel,
        "Oversampling",
        12.0f,
        textColour);

    configureLabel(
        qualityLabel,
        "Processing Quality",
        12.0f,
        textColour);

    //==========================================================================
    // DISPLAY
    //==========================================================================

    configureLabel(
        displayTitle,
        "DISPLAY",
        20.0f,
        textColour);

    configureLabel(
        displayDescription,
        "Customize the appearance and visual feedback.",
        11.0f,
        mutedColour);

    configureLabel(
        appearanceLabel,
        "APPEARANCE",
        11.0f,
        accentColour);

    configureLabel(
        displayThemeLabel,
        "Theme",
        12.0f,
        textColour);

    configureLabel(
        displayScaleLabel,
        "UI Scale",
        12.0f,
        textColour);

    configureLabel(
        visualLabel,
        "VISUAL FEEDBACK",
        11.0f,
        accentColour);

    configureLabel(
        inputMeterLabel,
        "Show Input Meter",
        12.0f,
        textColour);

    configureLabel(
        outputMeterLabel,
        "Show Output Meter",
        12.0f,
        textColour);

    configureLabel(
        tooltipsLabel,
        "Show Tooltips",
        12.0f,
        textColour);

    //==========================================================================
    // PERFORMANCE
    //==========================================================================

    configureLabel(
        performanceTitle,
        "PERFORMANCE",
        20.0f,
        textColour);

    configureLabel(
        performanceDescription,
        "Control processing quality and CPU behaviour.",
        11.0f,
        mutedColour);

    configureLabel(
        engineLabel,
        "ENGINE",
        11.0f,
        accentColour);

    configureLabel(
        cpuLabel,
        "CPU Mode",
        12.0f,
        textColour);

    configureLabel(
        processingQualityLabel,
        "Processing Quality",
        12.0f,
        textColour);

    //==========================================================================
    // ABOUT
    //==========================================================================

    configureLabel(
        aboutTitle,
        "ABOUT",
        20.0f,
        textColour);

    configureLabel(
        aboutDescription,
        "Information about Offor Vocal Pro.",
        11.0f,
        mutedColour);

    configureLabel(
        productNameLabel,
        "OFFOR VOCAL PRO",
        24.0f,
        textColour,
        juce::Justification::centred);

    configureLabel(
        versionLabel,
        "Version 1.0.0",
        11.0f,
        mutedColour,
        juce::Justification::centred);

    configureLabel(
        companyLabel,
        "Created by ONNTECH",
        12.0f,
        textColour,
        juce::Justification::centred);

    configureLabel(
        descriptionLabel,
        "Professional vocal processing for modern music.",
        11.0f,
        mutedColour,
        juce::Justification::centred);

    configureLabel(
        licenseTitleLabel,
        "LICENSE STATUS",
        11.0f,
        accentColour);

    configureLabel(
        licenseStatusLabel,
        "●  ACTIVATED",
        12.0f,
        juce::Colour(0xff65c98a));
}

//==============================================================================
// SETUP CONTROLS
//==============================================================================

void SettingsPanel::setupControls()
{
    //==========================================================================
    // GENERAL
    //==========================================================================

    addAndMakeVisible(enableProcessingToggle);

    addAndMakeVisible(uiScaleSelector);
    addAndMakeVisible(themeSelector);

    uiScaleSelector.setValue("100%");
    themeSelector.setValue("Dark");

    //==========================================================================
    // AUDIO
    //==========================================================================

    addAndMakeVisible(oversamplingSelector);
    addAndMakeVisible(qualitySelector);

    oversamplingSelector.setValue("2X");
    qualitySelector.setValue("High");

    //==========================================================================
    // DISPLAY
    //==========================================================================

    addAndMakeVisible(inputMeterToggle);
    addAndMakeVisible(outputMeterToggle);
    addAndMakeVisible(tooltipsToggle);

    addAndMakeVisible(displayThemeSelector);
    addAndMakeVisible(displayScaleSelector);

    inputMeterToggle.setToggleState(
        true,
        juce::dontSendNotification);

    outputMeterToggle.setToggleState(
        true,
        juce::dontSendNotification);

    tooltipsToggle.setToggleState(
        true,
        juce::dontSendNotification);

    displayThemeSelector.setValue("Dark");
    displayScaleSelector.setValue("100%");
}

//==============================================================================
// ABOUT PAGE
//==============================================================================

void SettingsPanel::setupAboutPage()
{
    addAndMakeVisible(websiteButton);
    addAndMakeVisible(supportButton);

    websiteButton.setButtonText(
        "VISIT WEBSITE");

    supportButton.setButtonText(
        "SUPPORT");

    websiteButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    supportButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    //--------------------------------------------------------------------------
    // These URLs can be connected later.
    //--------------------------------------------------------------------------

    websiteButton.onClick =
        []
        {
            juce::URL(
                "https://chechris.com"
            ).launchInDefaultBrowser();
        };

    supportButton.onClick =
        []
        {
            juce::URL(
                "https://ko-fi.com/"
            ).launchInDefaultBrowser();
        };
}

//==============================================================================
// CONFIGURE LABEL
//==============================================================================

void SettingsPanel::configureLabel(
    juce::Label& label,
    const juce::String& text,
    float fontSize,
    juce::Colour colour,
    juce::Justification justification)
{
    addAndMakeVisible(label);

    label.setText(
        text,
        juce::dontSendNotification);

    label.setColour(
        juce::Label::textColourId,
        colour);

    label.setJustificationType(
        justification);

    label.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(fontSize)
                .withStyle("Medium")));
}

//==============================================================================
// SELECT PAGE
//==============================================================================

void SettingsPanel::selectPage(Page page)
{
    currentPage = page;

    generalTab.setSelected(
        page == Page::general);

    audioTab.setSelected(
        page == Page::audio);

    displayTab.setSelected(
        page == Page::display);

    performanceTab.setSelected(
        page == Page::performance);

    aboutTab.setSelected(
        page == Page::about);

    updatePageVisibility();

    repaint();
}

//==============================================================================
// UPDATE PAGE VISIBILITY
//==============================================================================

void SettingsPanel::updatePageVisibility()
{
    const bool general =
        currentPage == Page::general;

    const bool audio =
        currentPage == Page::audio;

    const bool display =
        currentPage == Page::display;

    const bool performance =
        currentPage == Page::performance;

    const bool about =
        currentPage == Page::about;

    //==========================================================================
    // GENERAL
    //==========================================================================

    generalTitle.setVisible(general);
    generalDescription.setVisible(general);
    pluginBehaviourLabel.setVisible(general);
    enableProcessingLabel.setVisible(general);
    enableProcessingToggle.setVisible(general);
    interfaceLabel.setVisible(general);
    uiScaleLabel.setVisible(general);
    themeLabel.setVisible(general);
    uiScaleSelector.setVisible(general);
    themeSelector.setVisible(general);

    //==========================================================================
    // AUDIO
    //==========================================================================

    audioTitle.setVisible(audio);
    audioDescription.setVisible(audio);
    inputBehaviourLabel.setVisible(audio);
    inputGainLabel.setVisible(audio);
    processingLabel.setVisible(audio);
    oversamplingLabel.setVisible(audio);
    qualityLabel.setVisible(audio);
    oversamplingSelector.setVisible(audio);
    qualitySelector.setVisible(audio);

    //==========================================================================
    // DISPLAY
    //==========================================================================

    displayTitle.setVisible(display);
    displayDescription.setVisible(display);
    appearanceLabel.setVisible(display);
    displayThemeLabel.setVisible(display);
    displayScaleLabel.setVisible(display);
    visualLabel.setVisible(display);
    inputMeterLabel.setVisible(display);
    outputMeterLabel.setVisible(display);
    tooltipsLabel.setVisible(display);
    inputMeterToggle.setVisible(display);
    outputMeterToggle.setVisible(display);
    tooltipsToggle.setVisible(display);
    displayThemeSelector.setVisible(display);
    displayScaleSelector.setVisible(display);

    //==========================================================================
    // PERFORMANCE
    //==========================================================================

    performanceTitle.setVisible(performance);
    performanceDescription.setVisible(performance);
    engineLabel.setVisible(performance);
    cpuLabel.setVisible(performance);
    processingQualityLabel.setVisible(performance);
    cpuModeSelector.setVisible(performance);
    processingQualitySelector.setVisible(performance);

    //==========================================================================
    // ABOUT
    //==========================================================================

    aboutTitle.setVisible(about);
    aboutDescription.setVisible(about);
    productNameLabel.setVisible(about);
    versionLabel.setVisible(about);
    companyLabel.setVisible(about);
    descriptionLabel.setVisible(about);
    licenseTitleLabel.setVisible(about);
    licenseStatusLabel.setVisible(about);
    websiteButton.setVisible(about);
    supportButton.setVisible(about);
}

//==============================================================================
// DRAW PAGE HEADER
//==============================================================================

void SettingsPanel::drawPageHeader(
    juce::Graphics& g,
    const juce::String& title,
    const juce::String& description)
{
    juce::ignoreUnused(
        g,
        title,
        description);
}

//==============================================================================
// DRAW SECTION LINE
//==============================================================================

void SettingsPanel::drawSectionLine(
    juce::Graphics& g,
    int y)
{
    g.setColour(
        borderColour);

    g.drawHorizontalLine(
        y,
        40.0f,
        static_cast<float>(
            getWidth() - 40));
}

//==============================================================================
// DRAW SETTING ROW
//==============================================================================

void SettingsPanel::drawSettingRow(
    juce::Graphics& g,
    const juce::String& title,
    const juce::String& description,
    int y,
    int width)
{
    juce::ignoreUnused(
        g,
        title,
        description,
        y,
        width);
}

//==============================================================================
// PAINT
//==============================================================================

void SettingsPanel::paint(
    juce::Graphics& g)
{
    //==========================================================================
    // Entire background
    //==========================================================================

    g.fillAll(
        backgroundColour);

    auto bounds =
        getLocalBounds().toFloat();

    //==========================================================================
    // Main settings panel
    //==========================================================================

    auto panel =
        bounds.reduced(1.0f);

    g.setColour(
        panelColour);

    g.fillRoundedRectangle(
        panel,
        12.0f);

    //==========================================================================
    // Border
    //==========================================================================

    g.setColour(
        borderColour);

    g.drawRoundedRectangle(
        panel.reduced(0.5f),
        12.0f,
        1.0f);

    //==========================================================================
    // Header separator
    //==========================================================================

    g.setColour(
        borderColour);

    g.drawHorizontalLine(
        70,
        20.0f,
        static_cast<float>(
            getWidth() - 20));

    //==========================================================================
    // Header title
    //==========================================================================

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(16.0f)
                .withStyle("Bold")));

    g.setColour(
        textColour);

    g.drawText(
        "SETTINGS",
        28,
        16,
        180,
        22,
        juce::Justification::centredLeft,
        false);

    //==========================================================================
    // Product subtitle
    //==========================================================================

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(9.0f)
                .withStyle("Medium")));

    g.setColour(
        mutedColour);

    g.drawText(
        "OFFOR VOCAL PRO",
        29,
        39,
        180,
        16,
        juce::Justification::centredLeft,
        false);

    //==========================================================================
    // Navigation separator
    //==========================================================================

    g.setColour(
        borderColour);

    g.drawHorizontalLine(
        115,
        20.0f,
        static_cast<float>(
            getWidth() - 20));

    //==========================================================================
    // Page content background
    //==========================================================================

    g.setColour(
        panelColour.withAlpha(0.55f));

    g.fillRoundedRectangle(
        20.0f,
        126.0f,
        static_cast<float>(getWidth() - 40),
        static_cast<float>(getHeight() - 146),
        8.0f);

    //==========================================================================
    // Subtle page border
    //==========================================================================

    g.setColour(
        borderColour.withAlpha(0.45f));

    g.drawRoundedRectangle(
        20.0f,
        126.0f,
        static_cast<float>(getWidth() - 40),
        static_cast<float>(getHeight() - 146),
        8.0f,
        1.0f);

    //==========================================================================
    // General page
    //==========================================================================

    if (currentPage == Page::general)
    {
        g.setColour(borderColour);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(getWidth() - 40));

        g.drawHorizontalLine(
            365,
            40.0f,
            static_cast<float>(getWidth() - 40));
    }

    //==========================================================================
    // Audio page
    //==========================================================================

    if (currentPage == Page::audio)
    {
        g.setColour(borderColour);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(getWidth() - 40));

        g.drawHorizontalLine(
            365,
            40.0f,
            static_cast<float>(getWidth() - 40));
    }

    //==========================================================================
    // Display page
    //==========================================================================

    if (currentPage == Page::display)
    {
        g.setColour(borderColour);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(getWidth() - 40));

        g.drawHorizontalLine(
            365,
            40.0f,
            static_cast<float>(getWidth() - 40));
    }

    //==========================================================================
    // Performance page
    //==========================================================================

    if (currentPage == Page::performance)
    {
        g.setColour(borderColour);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(getWidth() - 40));
    }

    //==========================================================================
    // About page
    //==========================================================================

    if (currentPage == Page::about)
    {
        g.setColour(borderColour);

        g.drawHorizontalLine(
            360,
            70.0f,
            static_cast<float>(getWidth() - 70));
    }
}

//==============================================================================
// RESIZED
//==============================================================================

void SettingsPanel::resized()
{
    const int width = getWidth();
    const int height = getHeight();

    //==========================================================================
    // HEADER
    //==========================================================================

    closeButton.setBounds(
        width - 52,
        15,
        30,
        30);

    //==========================================================================
    // TOP NAVIGATION
    //
    // This is intentionally horizontal.
    // It gives Settings its own identity and keeps the main plugin's
    // module navigation completely separate.
    //==========================================================================

    const int navigationY = 77;
    const int navigationHeight = 35;

    const int navigationLeft = 35;
    const int navigationRight = width - 35;

    const int navigationWidth =
        navigationRight - navigationLeft;

    const int tabWidth =
        navigationWidth / 5;

    generalTab.setBounds(
        navigationLeft + tabWidth * 0,
        navigationY,
        tabWidth,
        navigationHeight);

    audioTab.setBounds(
        navigationLeft + tabWidth * 1,
        navigationY,
        tabWidth,
        navigationHeight);

    displayTab.setBounds(
        navigationLeft + tabWidth * 2,
        navigationY,
        tabWidth,
        navigationHeight);

    performanceTab.setBounds(
        navigationLeft + tabWidth * 3,
        navigationY,
        tabWidth,
        navigationHeight);

    aboutTab.setBounds(
        navigationLeft + tabWidth * 4,
        navigationY,
        tabWidth,
        navigationHeight);

    //==========================================================================
    // PAGE CONTENT
    //==========================================================================

    const int left = 50;
    const int right = width - 50;

    const int selectorWidth = 150;
    const int selectorHeight = 34;

    //==========================================================================
    // GENERAL
    //==========================================================================

    generalTitle.setBounds(
        left,
        150,
        300,
        30);

    generalDescription.setBounds(
        left,
        181,
        right - left,
        22);

    pluginBehaviourLabel.setBounds(
        left,
        220,
        220,
        20);

    enableProcessingLabel.setBounds(
        left,
        270,
        220,
        25);

    enableProcessingToggle.setBounds(
        right - 90,
        264,
        70,
        30);

    interfaceLabel.setBounds(
        left,
        335,
        220,
        20);

    uiScaleLabel.setBounds(
        left,
        385,
        220,
        25);

    uiScaleSelector.setBounds(
        right - selectorWidth,
        380,
        selectorWidth,
        selectorHeight);

    themeLabel.setBounds(
        left,
        440,
        220,
        25);

    themeSelector.setBounds(
        right - selectorWidth,
        435,
        selectorWidth,
        selectorHeight);

    //==========================================================================
    // AUDIO
    //==========================================================================

    audioTitle.setBounds(
        left,
        150,
        300,
        30);

    audioDescription.setBounds(
        left,
        181,
        right - left,
        22);

    inputBehaviourLabel.setBounds(
        left,
        220,
        250,
        20);

    inputGainLabel.setBounds(
        left,
        270,
        220,
        25);

    processingLabel.setBounds(
        left,
        335,
        220,
        20);

    oversamplingLabel.setBounds(
        left,
        385,
        220,
        25);

    oversamplingSelector.setBounds(
        right - selectorWidth,
        380,
        selectorWidth,
        selectorHeight);

    qualityLabel.setBounds(
        left,
        440,
        220,
        25);

    qualitySelector.setBounds(
        right - selectorWidth,
        435,
        selectorWidth,
        selectorHeight);

    //==========================================================================
    // DISPLAY
    //==========================================================================

    displayTitle.setBounds(
        left,
        150,
        300,
        30);

    displayDescription.setBounds(
        left,
        181,
        right - left,
        22);

    appearanceLabel.setBounds(
        left,
        220,
        220,
        20);

    displayThemeLabel.setBounds(
        left,
        270,
        220,
        25);

    displayThemeSelector.setBounds(
        right - selectorWidth,
        265,
        selectorWidth,
        selectorHeight);

    displayScaleLabel.setBounds(
        left,
        320,
        220,
        25);

    displayScaleSelector.setBounds(
        right - selectorWidth,
        315,
        selectorWidth,
        selectorHeight);

    visualLabel.setBounds(
        left,
        390,
        220,
        20);

    inputMeterLabel.setBounds(
        left,
        440,
        220,
        25);

    inputMeterToggle.setBounds(
        right - 90,
        434,
        70,
        30);

    outputMeterLabel.setBounds(
        left,
        490,
        220,
        25);

    outputMeterToggle.setBounds(
        right - 90,
        484,
        70,
        30);

    tooltipsLabel.setBounds(
        left,
        540,
        220,
        25);

    tooltipsToggle.setBounds(
        right - 90,
        534,
        70,
        30);

    //==========================================================================
    // PERFORMANCE
    //==========================================================================

    performanceTitle.setBounds(
        left,
        150,
        300,
        30);

    performanceDescription.setBounds(
        left,
        181,
        right - left,
        22);

    engineLabel.setBounds(
        left,
        220,
        220,
        20);

    cpuLabel.setBounds(
        left,
        270,
        220,
        25);

    cpuModeSelector.setBounds(
        right - selectorWidth,
        265,
        selectorWidth,
        selectorHeight);

    processingQualityLabel.setBounds(
        left,
        330,
        220,
        25);

    processingQualitySelector.setBounds(
        right - selectorWidth,
        325,
        selectorWidth,
        selectorHeight);

    //==========================================================================
    // ABOUT
    //==========================================================================

    productNameLabel.setBounds(
        left,
        170,
        right - left,
        35);

    versionLabel.setBounds(
        left,
        208,
        right - left,
        20);

    companyLabel.setBounds(
        left,
        245,
        right - left,
        24);

    descriptionLabel.setBounds(
        left,
        275,
        right - left,
        24);

    licenseTitleLabel.setBounds(
        left,
        390,
        250,
        20);

    licenseStatusLabel.setBounds(
        left,
        425,
        250,
        25);

    websiteButton.setBounds(
        left,
        480,
        155,
        38);

    supportButton.setBounds(
        left + 170,
        480,
        120,
        38);
}