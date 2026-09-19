#include "SettingsPanel.h"
#include "ThemeManager.h"
#include "../Version.h"

//==============================================================================
// LEGACY COLOURS
//==============================================================================
//
// These remain temporarily so existing code that references
// SettingsPanel::accentColour etc. continues to compile.
//
// Actual runtime painting now obtains colours from ThemeManager.
//
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

SettingsPanel::TabButton::TabButton(
    const juce::String& text)
    : juce::Button(text),
      buttonText(text)
{
    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);
}

//==============================================================================

void SettingsPanel::TabButton::setSelected(
    bool shouldBeSelected)
{
    selected = shouldBeSelected;
    repaint();
}

//==============================================================================

void SettingsPanel::TabButton::paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(
        shouldDrawButtonAsDown);

    const auto colours =
        ThemeManager::get().getColours();

    auto bounds =
        getLocalBounds().toFloat();

    if (shouldDrawButtonAsHighlighted && !selected)
    {
        g.setColour(
            colours.panel2.withAlpha(0.55f));

        g.fillRoundedRectangle(
            bounds.reduced(2.0f),
            6.0f);
    }

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)
                .withStyle("Bold")));

    g.setColour(
        selected
            ? colours.text
            : colours.muted);

    g.drawText(
        buttonText,
        getLocalBounds().reduced(4, 0),
        juce::Justification::centred,
        false);

    if (selected)
    {
        g.setColour(
            colours.accent);

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
    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);
}

//==============================================================================

void SettingsPanel::CloseButton::paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(
        shouldDrawButtonAsDown);

    const auto colours =
        ThemeManager::get().getColours();

    auto bounds =
        getLocalBounds().toFloat();

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(
            colours.panel2);

        g.fillRoundedRectangle(
            bounds.reduced(1.0f),
            7.0f);
    }

    g.setColour(
        shouldDrawButtonAsHighlighted
            ? colours.text
            : colours.muted);

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

    juce::Button::setToggleState(
        true,
        juce::dontSendNotification);

    isOn = true;

    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    setRepaintsOnMouseActivity(true);
}

//==============================================================================

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

//==============================================================================

bool SettingsPanel::ToggleSwitch::getToggleState() const
{
    return juce::Button::getToggleState();
}

//==============================================================================

void SettingsPanel::ToggleSwitch::paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(
        shouldDrawButtonAsDown);

    const auto colours =
        ThemeManager::get().getColours();

    auto bounds =
        getLocalBounds().toFloat();

    const float width =
        bounds.getWidth();

    const float height =
        24.0f;

    auto track =
        juce::Rectangle<float>(
            0.0f,
            (bounds.getHeight() - height) * 0.5f,
            width,
            height);

    const bool on =
        juce::Button::getToggleState();

    //--------------------------------------------------------------------------
    // Track
    //--------------------------------------------------------------------------

    g.setColour(
        on
            ? colours.accent
            : colours.border);

    g.fillRoundedRectangle(
        track,
        height * 0.5f);

    //--------------------------------------------------------------------------
    // Hover highlight
    //--------------------------------------------------------------------------

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(
            colours.text.withAlpha(0.08f));

        g.fillRoundedRectangle(
            track,
            height * 0.5f);
    }

    //--------------------------------------------------------------------------
    // Knob
    //--------------------------------------------------------------------------

    const float knobSize =
        height - 6.0f;

    const float knobX =
        on
            ? track.getRight() - knobSize - 3.0f
            : track.getX() + 3.0f;

    auto knob =
        juce::Rectangle<float>(
            knobX,
            track.getY() + 3.0f,
            knobSize,
            knobSize);

    g.setColour(
        colours.text);

    g.fillEllipse(knob);

    //--------------------------------------------------------------------------
    // ON / OFF
    //--------------------------------------------------------------------------

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(8.0f)
                .withStyle("Bold")));

    g.setColour(
        on
            ? colours.text
            : colours.muted);

    g.drawText(
        on ? "ON" : "OFF",
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

    setInterceptsMouseClicks(
        true,
        true);
}

//==============================================================================

void SettingsPanel::SettingSelector::setValue(
    const juce::String& newValue)
{
    value = newValue;
    repaint();
}

//==============================================================================

juce::String
SettingsPanel::SettingSelector::getValue() const
{
    return value;
}

//==============================================================================

void SettingsPanel::SettingSelector::setOptions(
    const juce::StringArray& newOptions)
{
    options = newOptions;

    if (!options.isEmpty() &&
        !options.contains(value))
    {
        value = options[0];
    }

    repaint();
}

//==============================================================================

void SettingsPanel::SettingSelector::showMenu()
{
    if (options.isEmpty())
        return;

    juce::PopupMenu menu;

    for (int i = 0; i < options.size(); ++i)
    {
        menu.addItem(
            i + 1,
            options[i],
            true,
            options[i] == value);
    }

    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetComponent(this)
            .withMinimumWidth(getWidth())
            .withMaximumNumColumns(1)
            .withStandardItemHeight(34),
        [this](int result)
        {
            if (result <= 0)
                return;

            const int index =
                result - 1;

            if (juce::isPositiveAndBelow(
                    index,
                    options.size()))
            {
                value =
                    options[index];

                repaint();

                if (onClicked)
                    onClicked();
            }
        });
}


//==============================================================================
// SETTING SELECTOR - MOUSE ENTER
//==============================================================================
//
// Highlights the selector when the mouse enters it.
//
//==============================================================================

void SettingsPanel::SettingSelector::mouseEnter(
    const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);

    mouseOver = true;

    repaint();
}


//==============================================================================
// SETTING SELECTOR - MOUSE EXIT
//==============================================================================
//
// Removes the hover highlight when the mouse leaves the selector.
//
//==============================================================================

void SettingsPanel::SettingSelector::mouseExit(
    const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);

    mouseOver = false;

    repaint();
}


//==============================================================================
// SETTING SELECTOR - MOUSE WHEEL
//==============================================================================
//
// Allows the selector to respond to mouse-wheel interaction.
//
// The actual menu selection is still handled by showMenu().
// We intentionally do not change the selected value here.
//
//==============================================================================

void SettingsPanel::SettingSelector::mouseWheelMove(
    const juce::MouseEvent& event,
    const juce::MouseWheelDetails& wheel)
{
    juce::ignoreUnused(event);
    juce::ignoreUnused(wheel);

    // Keep the selector behaviour unchanged.
    //
    // The mouse wheel is intentionally ignored here.
    // This implementation exists because the function is declared
    // in SettingsPanel.h.
}


//==============================================================================

void SettingsPanel::SettingSelector::mouseDown(
    const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);

    showMenu();
}

//==============================================================================

void SettingsPanel::SettingSelector::paint(
    juce::Graphics& g)
{
    const auto colours =
        ThemeManager::get().getColours();

    auto bounds =
        getLocalBounds().toFloat();

    g.setColour(
        mouseOver
            ? colours.panel2
            : colours.panel);

    g.fillRoundedRectangle(
        bounds,
        7.0f);

    g.setColour(
        mouseOver
            ? colours.accentDark
            : colours.border);

    g.drawRoundedRectangle(
        bounds.reduced(0.5f),
        7.0f,
        1.0f);

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)
                .withStyle("Medium")));

    g.setColour(
        colours.text);

    g.drawText(
        value,
        bounds.reduced(12.0f, 0.0f)
             .withRightX(
                 bounds.getRight() - 25.0f),
        juce::Justification::centredLeft,
        false);

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
        mouseOver
            ? colours.text
            : colours.muted);

    g.strokePath(
        arrow,
        juce::PathStrokeType(
            1.4f,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));
}

//==============================================================================

void SettingsPanel::SettingSelector::resized()
{
}

//==============================================================================
// CONSTRUCTOR
//==============================================================================

SettingsPanel::SettingsPanel()
{
    setOpaque(true);

    //==========================================================================
    // REGISTER AS A THEME LISTENER
    //==========================================================================

    ThemeManager::get().addChangeListener(this);

    //==========================================================================
    // CREATE UI
    //==========================================================================

    setupTabs();
    setupLabels();
    setupControls();
    setupAboutPage();
    setupUserGuide();

    //==========================================================================
    // FEEDBACK
    //==========================================================================
    //
    // Feedback is a separate component so the network/UI code does not
    // make SettingsPanel unnecessarily large.
    //
    //==========================================================================

    feedbackPanel =
        std::make_unique<FeedbackPanel>();

    addAndMakeVisible(
        *feedbackPanel);

    feedbackPanel->setVisible(false);

    // IMPORTANT:
    // Set callbacks AFTER controls have been created.
    setupSettingCallbacks();

    selectPage(Page::general);

    //==========================================================================
    // AUTO-SCROLL TIMER
    //==========================================================================
    //
    // 30 updates per second gives us very smooth automatic scrolling.
    //
    // The actual scroll amount is deliberately tiny so the guide feels
    // like a professional product manual rather than a fast marquee.
    //
    //==========================================================================

    startTimerHz(30);

    //==========================================================================
    // APPLY CURRENT THEME
    //==========================================================================

    updateThemeColours();
}

//==============================================================================
// DESTRUCTOR
//==============================================================================

SettingsPanel::~SettingsPanel()
{
    //==========================================================================
    // STOP TIMER
    //==========================================================================

    stopTimer();

    //==========================================================================
    // REMOVE LISTENER BEFORE DESTRUCTION
    //==========================================================================

    ThemeManager::get().removeChangeListener(this);
}

//==============================================================================
// SETTING CALLBACKS
//==============================================================================

void SettingsPanel::setupSettingCallbacks()
{
    //==========================================================================
    // ENABLE PROCESSING
    //==========================================================================

    enableProcessingToggle.onClick =
        [this]()
        {
            if (onProcessingChanged)
            {
                onProcessingChanged(
                    enableProcessingToggle.getToggleState());
            }
        };

    //==========================================================================
    // UI SCALE
    //==========================================================================

    uiScaleSelector.onClicked =
        [this]()
        {
            if (onUIScaleChanged)
            {
                onUIScaleChanged(
                    uiScaleSelector.getValue());
            }
        };

    //==========================================================================
    // THEME
    //==========================================================================

    themeSelector.onClicked =
        [this]()
        {
            const auto theme =
                themeSelector.getValue();

            syncThemeSelectors(theme);

            if (onThemeChanged)
                onThemeChanged(theme);

            if (onDisplayThemeChanged)
                onDisplayThemeChanged(theme);
        };

    //==========================================================================
    // DISPLAY THEME
    //==========================================================================

    displayThemeSelector.onClicked =
        [this]()
        {
            const auto theme =
                displayThemeSelector.getValue();

            syncThemeSelectors(theme);

            if (onThemeChanged)
                onThemeChanged(theme);

            if (onDisplayThemeChanged)
                onDisplayThemeChanged(theme);
        };

    //==========================================================================
    // OVERSAMPLING
    //==========================================================================

    oversamplingSelector.onClicked =
        [this]()
        {
            if (onOversamplingChanged)
            {
                onOversamplingChanged(
                    oversamplingSelector.getValue());
            }
        };

    //==========================================================================
    // PROCESSING QUALITY
    //==========================================================================

    qualitySelector.onClicked =
        [this]()
        {
            const auto quality =
                qualitySelector.getValue();

            syncProcessingQualitySelectors(
                quality);

            if (onProcessingQualityChanged)
                onProcessingQualityChanged(quality);
        };

    processingQualitySelector.onClicked =
        [this]()
        {
            const auto quality =
                processingQualitySelector.getValue();

            syncProcessingQualitySelectors(
                quality);

            if (onProcessingQualityChanged)
                onProcessingQualityChanged(quality);
        };

    //==========================================================================
    // INPUT METER
    //==========================================================================

    inputMeterToggle.onClick =
        [this]()
        {
            if (onInputMeterChanged)
            {
                onInputMeterChanged(
                    inputMeterToggle.getToggleState());
            }
        };

    //==========================================================================
    // OUTPUT METER
    //==========================================================================

    outputMeterToggle.onClick =
        [this]()
        {
            if (onOutputMeterChanged)
            {
                onOutputMeterChanged(
                    outputMeterToggle.getToggleState());
            }
        };

    //==========================================================================
    // TOOLTIPS
    //==========================================================================

    tooltipsToggle.onClick =
        [this]()
        {
            if (onTooltipsChanged)
            {
                onTooltipsChanged(
                    tooltipsToggle.getToggleState());
            }
        };

    //==========================================================================
    // DISPLAY SCALE
    //==========================================================================

    displayScaleSelector.onClicked =
        [this]()
        {
            if (onDisplayScaleChanged)
            {
                onDisplayScaleChanged(
                    displayScaleSelector.getValue());
            }
        };

    //==========================================================================
    // CPU MODE
    //==========================================================================

    cpuModeSelector.onClicked =
        [this]()
        {
            if (onCPUModeChanged)
            {
                onCPUModeChanged(
                    cpuModeSelector.getValue());
            }
        };
}

//==============================================================================
// THEME CONTROLS
//==============================================================================

void SettingsPanel::setupThemeControls()
{
    const auto themeNames =
        ThemeManager::getThemeNames();

    themeSelector.setOptions(
        themeNames);

    displayThemeSelector.setOptions(
        themeNames);

    const auto currentTheme =
        ThemeManager::get().getThemeName();

    themeSelector.setValue(
        currentTheme);

    displayThemeSelector.setValue(
        currentTheme);

    customizeColoursButton.setButtonText(
        "CUSTOMIZE COLOURS");

    customizeColoursButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    customizeColoursButton.setTooltip(
        "Create your own custom OFFOR Vocal Pro colour theme.");

    customizeColoursButton.onClick =
        [this]()
        {
            showThemeColorPanel();
        };

    addAndMakeVisible(
        customizeColoursButton);
}

//==============================================================================
// SHOW THEME COLOR PANEL
//==============================================================================

void SettingsPanel::showThemeColorPanel()
{
    if (themeColorPanel == nullptr)
    {
        themeColorPanel =
            std::make_unique<ThemeColorPanel>();

        themeColorPanel->onClose =
            [this]()
            {
                hideThemeColorPanel();
            };
    }

    addAndMakeVisible(
        *themeColorPanel);

    themeColorPanel->updateDisplay();

    themeColorPanel->toFront(true);

    customColoursVisible = true;

    themeColorPanel->setBounds(
        getLocalBounds().reduced(25));

    repaint();
}

//==============================================================================
// HIDE THEME COLOR PANEL
//==============================================================================

void SettingsPanel::hideThemeColorPanel()
{
    if (themeColorPanel != nullptr)
        themeColorPanel->setVisible(false);

    customColoursVisible = false;

    repaint();
}

//==============================================================================
// THEME CHANGE LISTENER
//==============================================================================

void SettingsPanel::changeListenerCallback(
    juce::ChangeBroadcaster* source)
{
    if (source != &ThemeManager::get())
        return;

    const auto theme =
        ThemeManager::get().getThemeName();

    syncThemeSelectors(
        theme);

    updateThemeColours();

    if (themeColorPanel != nullptr)
        themeColorPanel->updateDisplay();

    repaint();
}

//==============================================================================
// UPDATE THEME COLOURS
//==============================================================================

void SettingsPanel::updateThemeColours()
{
    const auto colours =
        ThemeManager::get().getColours();

    //==========================================================================
    // LABELS
    //==========================================================================

    juce::Label* labels[] =
    {
        &generalTitle,
        &generalDescription,
        &pluginBehaviourLabel,
        &enableProcessingLabel,
        &interfaceLabel,
        &uiScaleLabel,
        &themeLabel,

        &audioTitle,
        &audioDescription,
        &inputBehaviourLabel,
        &inputGainLabel,
        &processingLabel,
        &oversamplingLabel,
        &qualityLabel,

        &displayTitle,
        &displayDescription,
        &appearanceLabel,
        &displayThemeLabel,
        &displayScaleLabel,
        &visualLabel,
        &inputMeterLabel,
        &outputMeterLabel,
        &tooltipsLabel,

        &performanceTitle,
        &performanceDescription,
        &engineLabel,
        &cpuLabel,
        &processingQualityLabel,

        &aboutTitle,
        &aboutDescription,
        &productNameLabel,
        &versionLabel,
        &companyLabel,
        &descriptionLabel,
        &licenseTitleLabel,

        &userGuideTitle,
        &userGuideDescription
    };

    for (auto* label : labels)
    {
        label->setColour(
            juce::Label::textColourId,
            colours.text);

        label->repaint();
    }

    //==========================================================================
    // DESCRIPTION / MUTED LABELS
    //==========================================================================

    generalDescription.setColour(
        juce::Label::textColourId,
        colours.muted);

    audioDescription.setColour(
        juce::Label::textColourId,
        colours.muted);

    displayDescription.setColour(
        juce::Label::textColourId,
        colours.muted);

    performanceDescription.setColour(
        juce::Label::textColourId,
        colours.muted);

    aboutDescription.setColour(
        juce::Label::textColourId,
        colours.muted);

    versionLabel.setColour(
        juce::Label::textColourId,
        colours.muted);

    descriptionLabel.setColour(
        juce::Label::textColourId,
        colours.muted);

    userGuideDescription.setColour(
        juce::Label::textColourId,
        colours.muted);

    //==========================================================================
    // SECTION LABELS
    //==========================================================================

    juce::Label* sectionLabels[] =
    {
        &pluginBehaviourLabel,
        &interfaceLabel,
        &inputBehaviourLabel,
        &processingLabel,
        &appearanceLabel,
        &visualLabel,
        &engineLabel,
        &licenseTitleLabel
    };

    for (auto* label : sectionLabels)
    {
        label->setColour(
            juce::Label::textColourId,
            colours.accent);

        label->repaint();
    }

    //==========================================================================
    // LICENSE STATUS
    //==========================================================================

    licenseStatusLabel.setColour(
        juce::Label::textColourId,
        colours.success);

    licenseStatusLabel.repaint();

    //==========================================================================
    // USER GUIDE
    //==========================================================================

    userGuideContent.setColour(
        juce::Label::textColourId,
        colours.text);

    userGuideContent.setColour(
        juce::Label::backgroundColourId,
        juce::Colours::transparentBlack);

    userGuideContent.repaint();

    //==========================================================================
    // BUTTONS
    //==========================================================================

    customizeColoursButton.setColour(
        juce::TextButton::buttonColourId,
        colours.panel2);

    customizeColoursButton.setColour(
        juce::TextButton::buttonOnColourId,
        colours.accentDark);

    customizeColoursButton.setColour(
        juce::TextButton::textColourOffId,
        colours.text);

    customizeColoursButton.setColour(
        juce::TextButton::textColourOnId,
        colours.text);

    customizeColoursButton.repaint();

    websiteButton.setColour(
        juce::TextButton::buttonColourId,
        colours.panel2);

    websiteButton.setColour(
        juce::TextButton::textColourOffId,
        colours.text);

    websiteButton.repaint();

    supportButton.setColour(
        juce::TextButton::buttonColourId,
        colours.panel2);

    supportButton.setColour(
        juce::TextButton::textColourOffId,
        colours.text);

    supportButton.repaint();

    //==========================================================================
    // TOGGLES
    //==========================================================================

    enableProcessingToggle.repaint();
    inputMeterToggle.repaint();
    outputMeterToggle.repaint();
    tooltipsToggle.repaint();

    //==========================================================================
    // SELECTORS
    //==========================================================================

    themeSelector.repaint();
    displayThemeSelector.repaint();

    uiScaleSelector.repaint();
    oversamplingSelector.repaint();
    qualitySelector.repaint();
    displayScaleSelector.repaint();
    cpuModeSelector.repaint();
    processingQualitySelector.repaint();

    //==========================================================================
    // NAVIGATION
    //==========================================================================

    generalTab.repaint();
    audioTab.repaint();
    displayTab.repaint();
    performanceTab.repaint();
    aboutTab.repaint();
    userGuideTab.repaint();

    closeButton.repaint();

    feedbackTab.repaint();

    if (feedbackPanel != nullptr)
        feedbackPanel->updateThemeColours();

    //==========================================================================
    // PANEL
    //==========================================================================

    repaint();
}

//==============================================================================
// SYNCHRONIZE THEME SELECTORS
//==============================================================================

void SettingsPanel::syncThemeSelectors(
    const juce::String& theme)
{
    themeSelector.setValue(theme);

    displayThemeSelector.setValue(theme);
}

//==============================================================================
// SYNCHRONIZE PROCESSING QUALITY
//==============================================================================

void SettingsPanel::syncProcessingQualitySelectors(
    const juce::String& quality)
{
    qualitySelector.setValue(quality);

    processingQualitySelector.setValue(
        quality);
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
    addAndMakeVisible(userGuideTab);
    addAndMakeVisible(feedbackTab);

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

    userGuideTab.onClick =
        [this]()
        {
            selectPage(Page::userGuide);
        };

    feedbackTab.onClick =
        [this]()
        {
            selectPage(Page::feedback);
        };

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
        "Version " + juce::String(OFFOR_VPRO_VERSION_STRING),
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
        "* ACTIVATED",
        12.0f,
        juce::Colour(0xff65c98a));

    //==========================================================================
    // USER GUIDE
    //==========================================================================

    configureLabel(
        userGuideTitle,
        "USER GUIDE",
        20.0f,
        textColour);

    configureLabel(
        userGuideDescription,
        "Learn the OFFOR Vocal Pro workflow, controls and recommended techniques.",
        11.0f,
        mutedColour);

    
    //==========================================================================
    // ADD ALL LABELS TO SETTINGS PANEL
    //==========================================================================
    //
    // IMPORTANT:
    // configureLabel() only configures a label.
    //
    // The label must also be added to SettingsPanel before it can actually
    // appear on screen.
    //
    // This was the reason the setting text/descriptions disappeared while
    // the combo boxes and switches remained visible.
    //
    //==========================================================================

    addAndMakeVisible(generalTitle);
    addAndMakeVisible(generalDescription);
    addAndMakeVisible(pluginBehaviourLabel);
    addAndMakeVisible(enableProcessingLabel);
    addAndMakeVisible(interfaceLabel);
    addAndMakeVisible(uiScaleLabel);
    addAndMakeVisible(themeLabel);

    addAndMakeVisible(audioTitle);
    addAndMakeVisible(audioDescription);
    addAndMakeVisible(inputBehaviourLabel);
    addAndMakeVisible(inputGainLabel);
    addAndMakeVisible(processingLabel);
    addAndMakeVisible(oversamplingLabel);
    addAndMakeVisible(qualityLabel);

    addAndMakeVisible(displayTitle);
    addAndMakeVisible(displayDescription);
    addAndMakeVisible(appearanceLabel);
    addAndMakeVisible(displayThemeLabel);
    addAndMakeVisible(displayScaleLabel);
    addAndMakeVisible(visualLabel);
    addAndMakeVisible(inputMeterLabel);
    addAndMakeVisible(outputMeterLabel);
    addAndMakeVisible(tooltipsLabel);

    addAndMakeVisible(performanceTitle);
    addAndMakeVisible(performanceDescription);
    addAndMakeVisible(engineLabel);
    addAndMakeVisible(cpuLabel);
    addAndMakeVisible(processingQualityLabel);

    addAndMakeVisible(aboutTitle);
    addAndMakeVisible(aboutDescription);
    addAndMakeVisible(productNameLabel);
    addAndMakeVisible(versionLabel);
    addAndMakeVisible(companyLabel);
    addAndMakeVisible(descriptionLabel);
    addAndMakeVisible(licenseTitleLabel);
    addAndMakeVisible(licenseStatusLabel);

    addAndMakeVisible(userGuideTitle);
    addAndMakeVisible(userGuideDescription);


}

//==============================================================================
// SETUP CONTROLS
//==============================================================================

void SettingsPanel::setupControls()
{
    //==========================================================================
    // GENERAL
    //==========================================================================

    addAndMakeVisible(
        enableProcessingToggle);

    addAndMakeVisible(
        uiScaleSelector);

    addAndMakeVisible(
        themeSelector);

    uiScaleSelector.setOptions(
    {
        "75%",
        "90%",
        "100%",
        "110%",
        "125%",
        "150%"
    });

    uiScaleSelector.setValue(
        "100%");

    //==========================================================================
    // THEME
    //==========================================================================

    setupThemeControls();

    //==========================================================================
    // AUDIO
    //==========================================================================

    addAndMakeVisible(
        oversamplingSelector);

    addAndMakeVisible(
        qualitySelector);

    oversamplingSelector.setOptions(
    {
        "Off",
        "2X",
        "4X",
        "8X"
    });

    oversamplingSelector.setValue(
        "2X");

    qualitySelector.setOptions(
    {
        "Low",
        "Medium",
        "High",
        "Ultra"
    });

    qualitySelector.setValue(
        "High");

    //==========================================================================
    // DISPLAY
    //==========================================================================

    addAndMakeVisible(
        inputMeterToggle);

    addAndMakeVisible(
        outputMeterToggle);

    addAndMakeVisible(
        tooltipsToggle);

    addAndMakeVisible(
        displayThemeSelector);

    addAndMakeVisible(
        displayScaleSelector);

    inputMeterToggle.setToggleState(
        true,
        juce::dontSendNotification);

    outputMeterToggle.setToggleState(
        true,
        juce::dontSendNotification);

    tooltipsToggle.setToggleState(
        true,
        juce::dontSendNotification);

    displayThemeSelector.setValue(
        ThemeManager::get().getThemeName());

    displayScaleSelector.setOptions(
    {
        "75%",
        "90%",
        "100%",
        "110%",
        "125%",
        "150%"
    });

    displayScaleSelector.setValue(
        "100%");

    //==========================================================================
    // PERFORMANCE
    //==========================================================================

    addAndMakeVisible(
        cpuModeSelector);

    addAndMakeVisible(
        processingQualitySelector);

    cpuModeSelector.setOptions(
    {
        "Balanced",
        "Performance",
        "Low CPU"
    });

    cpuModeSelector.setValue(
        "Balanced");

    processingQualitySelector.setOptions(
    {
        "Low",
        "Medium",
        "High",
        "Ultra"
    });

    processingQualitySelector.setValue(
        "High");
}

//==============================================================================
// ABOUT PAGE
//==============================================================================

void SettingsPanel::setupAboutPage()
{
    addAndMakeVisible(
        websiteButton);

    addAndMakeVisible(
        supportButton);

    websiteButton.setButtonText(
        "VISIT WEBSITE");

    supportButton.setButtonText(
        "SUPPORT");

    websiteButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    supportButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    websiteButton.onClick =
        []
        {
            juce::URL(
                "https://chechris.com/software")
                .launchInDefaultBrowser();
        };

    supportButton.onClick =
        []
        {
            juce::URL(
                "https://ko-fi.com/chechris")
                .launchInDefaultBrowser();
        };
}


//==============================================================================
// SETUP USER GUIDE
//==============================================================================
//
// The User Guide uses a JUCE Viewport containing a tall Label.
//
// IMPORTANT:
// The Viewport MUST be added to SettingsPanel.
// Without addAndMakeVisible(), setVisible(true) alone is not enough.
//
// Mouse behaviour:
//
//     Mouse outside:
//         Automatic slow scrolling.
//
//     Mouse enters:
//         Automatic scrolling pauses.
//
//     Mouse wheel:
//         Fast manual scrolling.
//
//     Mouse leaves:
//         Automatic scrolling resumes after a short delay.
//
//==============================================================================

// ============================================================
// USER GUIDE SETUP
// ============================================================

void SettingsPanel::setupUserGuide()
{
    // --------------------------------------------------------
    // ADD VIEWPORT TO SETTINGS PANEL
    // IMPORTANT: The viewport must be visible.
    // --------------------------------------------------------

    addAndMakeVisible(userGuideViewport);

    // --------------------------------------------------------
    // VIEWPORT SETTINGS
    // --------------------------------------------------------

    userGuideViewport.setScrollBarsShown(
        false,
        false);

    userGuideViewport.setScrollOnDragEnabled(
        true);

    userGuideViewport.setWantsKeyboardFocus(
        false);

    userGuideViewport.setMouseCursor(
        juce::MouseCursor::NormalCursor);

    // --------------------------------------------------------
    // CREATE THE CONTENT INSIDE THE VIEWPORT
    // --------------------------------------------------------

    userGuideViewport.setViewedComponent(
        &userGuideContent,
        false);

    // --------------------------------------------------------
    // GUIDE TEXT
    // --------------------------------------------------------

    userGuideContent.setText(
        getUserGuideText(),
        juce::dontSendNotification);

    userGuideContent.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.5f)
                .withStyle("Medium")));

    userGuideContent.setJustificationType(
        juce::Justification::topLeft);

    // --------------------------------------------------------
    // TEXT PADDING
    // --------------------------------------------------------

    userGuideContent.setBorderSize(
        juce::BorderSize<int>(
            18,     // top
            22,     // left
            30,     // bottom
            22));   // right

    // --------------------------------------------------------
    // IMPORTANT:
    // The LABEL does NOT capture mouse events.
    // This allows the VIEWPORT to receive the mouse wheel.
    // --------------------------------------------------------

    userGuideContent.setInterceptsMouseClicks(
        false,
        false);

    // --------------------------------------------------------
    // TRANSPARENT BACKGROUND
    // --------------------------------------------------------

    userGuideContent.setColour(
        juce::Label::backgroundColourId,
        juce::Colours::transparentBlack);

    // --------------------------------------------------------
    // MAKE SURE THE CONTENT IS VISIBLE
    // --------------------------------------------------------

    userGuideContent.setVisible(true);

    // --------------------------------------------------------
    // MOUSE ENTER
    // Stop automatic scrolling while mouse is inside guide.
    // --------------------------------------------------------

    userGuideViewport.onMouseEnterGuide =
        [this]()
        {
            userGuideMouseOver = true;
            userGuideIdleCounter = 0;
        };

    // --------------------------------------------------------
    // MOUSE EXIT
    // Automatic scrolling will resume after the delay.
    // --------------------------------------------------------

    userGuideViewport.onMouseExitGuide =
        [this]()
        {
            userGuideMouseOver = false;
            userGuideIdleCounter = 0;
        };

    // --------------------------------------------------------
    // MOUSE WHEEL
    // Fast manual scrolling.
    // --------------------------------------------------------

    userGuideViewport.onMouseWheelGuide =
        [this](
            const juce::MouseEvent& event,
            const juce::MouseWheelDetails& wheel)
        {
            juce::ignoreUnused(event);

            if (currentPage != Page::userGuide)
                return;

            const auto* content =
                userGuideViewport.getViewedComponent();

            if (content == nullptr)
                return;

            const int viewportHeight =
                userGuideViewport.getHeight();

            const int contentHeight =
                content->getHeight();

            const int maxScroll =
                juce::jmax(
                    0,
                    contentHeight - viewportHeight);

            if (maxScroll <= 0)
                return;

            // Mouse is interacting with the guide.
            userGuideMouseOver = true;
            userGuideIdleCounter = 0;

            // ------------------------------------------------
            // WHEEL SPEED
            // Increase this number for faster scrolling.
            // ------------------------------------------------

            constexpr float mouseWheelSpeed = 420.0f;

            float delta =
                wheel.deltaY * mouseWheelSpeed;

            // Some touchpads use horizontal delta.
            if (std::abs(delta) < 0.01f)
            {
                delta =
                    wheel.deltaX * mouseWheelSpeed;
            }

            const int currentPosition =
                userGuideViewport.getViewPositionY();

            const int newPosition =
                juce::jlimit(
                    0,
                    maxScroll,
                    currentPosition
                        - static_cast<int>(delta));

            userGuideScrollPosition =
                static_cast<float>(
                    newPosition);

            userGuideViewport.setViewPosition(
                0,
                newPosition);
        };

    // --------------------------------------------------------
    // INITIAL SCROLL STATE
    // --------------------------------------------------------

    userGuideMouseOver = false;
    userGuideIdleCounter = 0;
    userGuideScrollPosition = 0.0f;

    // --------------------------------------------------------
    // FORCE INITIAL LAYOUT
    // resized() will also call this later when the panel
    // receives its real size.
    // --------------------------------------------------------

    updateUserGuideLayout();

    resetUserGuideScroll();
}

// ============================================================
// USER GUIDE LAYOUT
// ============================================================

void SettingsPanel::updateUserGuideLayout()
{
    const int viewportWidth =
        userGuideViewport.getWidth();

    const int viewportHeight =
        userGuideViewport.getHeight();

    // The panel may not have been sized yet.
    if (viewportWidth <= 0 ||
        viewportHeight <= 0)
    {
        return;
    }

    // --------------------------------------------------------
    // CONTENT WIDTH
    // Make the label exactly the width of the viewport.
    // --------------------------------------------------------

    const int contentWidth =
        juce::jmax(
            100,
            viewportWidth);

    // --------------------------------------------------------
    // CONTENT HEIGHT
    //
    // This is deliberately much taller than the viewport so
    // the entire guide can scroll.
    // --------------------------------------------------------

    const int contentHeight = 6000;

    userGuideContent.setBounds(
        0,
        0,
        contentWidth,
        contentHeight);

    // --------------------------------------------------------
    // MAKE ABSOLUTELY SURE THE CONTENT IS VISIBLE.
    // --------------------------------------------------------

    userGuideContent.setVisible(true);

    userGuideContent.toBack();

    // --------------------------------------------------------
    // LIMIT CURRENT SCROLL POSITION.
    // --------------------------------------------------------

    const int maxScroll =
        juce::jmax(
            0,
            contentHeight - viewportHeight);

    const int currentPosition =
        juce::jlimit(
            0,
            maxScroll,
            userGuideViewport.getViewPositionY());

    userGuideScrollPosition =
        static_cast<float>(
            currentPosition);

    // --------------------------------------------------------
    // APPLY SCROLL POSITION.
    // --------------------------------------------------------

    userGuideViewport.setViewPosition(
        0,
        currentPosition);
}



//==============================================================================
// USER GUIDE CONTENT
//==============================================================================

juce::String SettingsPanel::getUserGuideText() const
{
    return
        "WELCOME TO OFFOR VOCAL PRO\n"
        "\n"
        "Professional vocal processing designed for fast, creative and "
        "controlled vocal production.\n"
        "\n"
        "OFFOR Vocal Pro gives you a complete environment for shaping a "
        "vocal from clean input to a finished production sound.\n"
        "\n"
        "\n"
        "GETTING STARTED\n"
        "\n"
        "Start with a clean vocal recording and place OFFOR Vocal Pro on "
        "your vocal track or vocal bus.\n"
        "\n"
        "Begin with the main controls and make small adjustments. "
        "The goal is not to use every control heavily. The goal is to "
        "build the sound you actually need.\n"
        "\n"
        "\n"
        "MAIN CONTROLS\n"
        "\n"
        "INPUT\n"
        "Controls the level entering the processing chain. Use it to "
        "set a sensible level before applying heavy processing.\n"
        "\n"
        "VOCAL BOOST\n"
        "Controls the overall intensity of the vocal enhancement. "
        "Increase it gradually until the vocal becomes more present "
        "without sounding unnatural.\n"
        "\n"
        "MIX\n"
        "Blends the processed vocal with the original signal. This is "
        "useful when you want the character of the processing while "
        "keeping some of the natural vocal.\n"
        "\n"
        "WARMTH\n"
        "Adds warmth and body to the vocal character.\n"
        "\n"
        "BODY\n"
        "Controls additional low-mid vocal presence and weight.\n"
        "\n"
        "AIR\n"
        "Adds high-frequency openness and vocal brightness.\n"
        "\n"
        "DRIVE\n"
        "Adds harmonic character and saturation. Use carefully for "
        "more aggressive or intimate vocal tones.\n"
        "\n"
        "COMPRESS\n"
        "Controls the amount of dynamic control applied to the vocal.\n"
        "\n"
        "DE-ESS\n"
        "Reduces excessive sibilance such as strong S, SH and T sounds.\n"
        "\n"
        "OUTPUT\n"
        "Controls the final output level after processing. Always check "
        "your output level when increasing processing intensity.\n"
        "\n"
        "\n"
        "TUNER\n"
        "\n"
        "The tuner section is designed to help control vocal pitch and "
        "pitch movement.\n"
        "\n"
        "RETUNE\n"
        "Controls how quickly the vocal pitch moves toward the detected "
        "target note. Faster settings produce a more obvious correction "
        "effect, while slower settings can retain more natural movement.\n"
        "\n"
        "SMOOTH\n"
        "Controls the smoothness of pitch movement. Use it to reduce "
        "unnatural or abrupt transitions.\n"
        "\n"
        "FORMANT\n"
        "Adjusts the vocal character independently from the detected "
        "pitch. This can change the perceived vocal character while "
        "keeping the pitch treatment separate.\n"
        "\n"
        "\n"
        "DOUBLER\n"
        "\n"
        "The doubler creates the impression of additional vocal takes "
        "around the original performance.\n"
        "\n"
        "AMOUNT\n"
        "Controls how much doubled voice is generated.\n"
        "\n"
        "DETUNE\n"
        "Controls pitch difference between the doubled voices.\n"
        "\n"
        "TIMING\n"
        "Controls timing variation between the original and doubled "
        "voices.\n"
        "\n"
        "WIDTH\n"
        "Controls stereo spread of the doubled voices.\n"
        "\n"
        "MIX\n"
        "Controls how much of the doubler is blended into the final sound.\n"
        "\n"
        "\n"
        "HARMONY\n"
        "\n"
        "Harmony creates additional vocal voices based on the selected "
        "intervals.\n"
        "\n"
        "VOICE 1 - 4\n"
        "Select the harmony interval for each additional voice.\n"
        "\n"
        "MIX\n"
        "Controls the overall harmony level.\n"
        "\n"
        "TIP\n"
        "Harmony normally works best when used as a supporting layer "
        "rather than overpowering the lead vocal.\n"
        "\n"
        "\n"
        "CREATIVE FX\n"
        "\n"
        "Creative FX are designed for additional vocal character and "
        "sound-design possibilities.\n"
        "\n"
        "Select an effect and adjust its AMOUNT and MIX controls.\n"
        "\n"
        "Use Creative FX for transitions, ad-libs, hooks, special vocal "
        "moments and other creative production elements.\n"
        "\n"
        "\n"
        "SPACE\n"
        "\n"
        "The Space section adds ambience and depth around the vocal.\n"
        "\n"
        "SIZE\n"
        "Controls the perceived size of the space.\n"
        "\n"
        "DECAY\n"
        "Controls how long the space continues after the vocal signal.\n"
        "\n"
        "PRE-DELAY\n"
        "Controls the delay before the space effect becomes audible. "
        "More pre-delay can help keep the lead vocal clear in front of "
        "the effect.\n"
        "\n"
        "DAMPING\n"
        "Controls high-frequency absorption inside the space.\n"
        "\n"
        "MIX\n"
        "Controls the amount of Space effect blended into the final sound.\n"
        "\n"
        "\n"
        "PRESETS\n"
        "\n"
        "Use the PRESET selector in the main header to choose a factory "
        "preset.\n"
        "\n"
        "To access preset file operations, open the small (...) menu beside "
        "the preset selector.\n"
        "\n"
        "LOAD PRESET\n"
        "Loads a previously saved OFFOR Vocal Pro preset file.\n"
        "\n"
        "SAVE PRESET\n"
        "Saves the current plugin state so it can be loaded again later.\n"
        "\n"
        "TIP\n"
        "Save useful vocal chains as presets. This can significantly speed "
        "up repetitive production work.\n"
        "\n"
        "\n"
        "A / B COMPARISON\n"
        "\n"
        "The A and B buttons allow quick comparison between two processing "
        "states.\n"
        "\n"
        "Use A/B while making important processing decisions. If the "
        "processed version sounds louder, remember that louder can easily "
        "be perceived as better.\n"
        "\n"
        "Match levels when comparing whenever possible.\n"
        "\n"
        "\n"
        "SETTINGS\n"
        "\n"
        "GENERAL\n"
        "\n"
        "ENABLE PROCESSING\n"
        "Turns the main processing engine on or off.\n"
        "\n"
        "UI SCALE\n"
        "Changes the size of the OFFOR Vocal Pro interface.\n"
        "\n"
        "THEME\n"
        "Selects the visual theme used by the plugin.\n"
        "\n"
        "CUSTOMIZE COLOURS\n"
        "Allows you to create your own custom OFFOR Vocal Pro colour theme.\n"
        "\n"
        "\n"
        "AUDIO\n"
        "\n"
        "OVERSAMPLING\n"
        "Higher oversampling can improve the quality of certain nonlinear "
        "processing operations, but may increase CPU usage.\n"
        "\n"
        "OFF\n"
        "Lowest additional CPU cost.\n"
        "\n"
        "2X\n"
        "A balanced oversampling setting for normal production work.\n"
        "\n"
        "4X\n"
        "Higher processing resolution with increased CPU usage.\n"
        "\n"
        "8X\n"
        "Highest available oversampling setting with the greatest CPU cost.\n"
        "\n"
        "PROCESSING QUALITY\n"
        "Controls the processing quality profile used by the plugin.\n"
        "\n"
        "\n"
        "DISPLAY\n"
        "\n"
        "THEME\n"
        "Controls the visual appearance of the plugin interface.\n"
        "\n"
        "UI SCALE\n"
        "Changes the size of the plugin interface.\n"
        "\n"
        "SHOW INPUT METER\n"
        "Shows or hides the input level meter.\n"
        "\n"
        "SHOW OUTPUT METER\n"
        "Shows or hides the output level meter.\n"
        "\n"
        "SHOW TOOLTIPS\n"
        "Enables or disables control tooltips.\n"
        "\n"
        "\n"
        "PERFORMANCE\n"
        "\n"
        "CPU MODE\n"
        "Controls the balance between CPU usage and processing performance.\n"
        "\n"
        "LOW CPU\n"
        "Designed for systems where processor usage needs to be minimized.\n"
        "\n"
        "BALANCED\n"
        "Designed as the normal everyday production mode.\n"
        "\n"
        "PERFORMANCE\n"
        "Prioritizes processing performance where additional CPU usage is "
        "acceptable.\n"
        "\n"
        "PROCESSING QUALITY\n"
        "Provides additional control over the processing quality profile.\n"
        "\n"
        "\n"
        "RECOMMENDED VOCAL WORKFLOW\n"
        "\n"
        "1. Load your vocal into your DAW.\n"
        "\n"
        "2. Insert OFFOR Vocal Pro on the vocal track or vocal bus.\n"
        "\n"
        "3. Set INPUT to a sensible level.\n"
        "\n"
        "4. Start with a small amount of VOCAL BOOST.\n"
        "\n"
        "5. Adjust WARMTH, BODY and AIR to shape the vocal character.\n"
        "\n"
        "6. Add COMPRESS to control vocal dynamics.\n"
        "\n"
        "7. Use DE-ESS if the vocal contains excessive sibilance.\n"
        "\n"
        "8. Use TUNER when pitch correction or pitch shaping is required.\n"
        "\n"
        "9. Add DOUBLER or HARMONY when additional vocal layers are needed.\n"
        "\n"
        "10. Use CREATIVE FX for special vocal moments.\n"
        "\n"
        "11. Add SPACE for depth and ambience.\n"
        "\n"
        "12. Adjust MIX and OUTPUT to keep the final result controlled.\n"
        "\n"
        "13. Compare your result using A/B.\n"
        "\n"
        "14. Save the finished chain as a preset when you have a sound you "
        "want to reuse.\n"
        "\n"
        "\n"
        "PRODUCTION TIPS\n"
        "\n"
        "START SMALL\n"
        "Small changes often sound more natural than extreme processing.\n"
        "\n"
        "USE MIX CONTROLS\n"
        "Parallel-style blending can help retain the natural character of "
        "the original vocal.\n"
        "\n"
        "WATCH YOUR LEVELS\n"
        "Avoid judging processing only by loudness. Match levels when "
        "comparing processed and unprocessed signals.\n"
        "\n"
        "USE A/B OFTEN\n"
        "Regular comparison helps you decide whether each processing stage "
        "is actually improving the vocal.\n"
        "\n"
        "WATCH CPU USAGE\n"
        "Higher processing quality, oversampling and complex processing "
        "can increase CPU usage.\n"
        "\n"
        "SAVE YOUR SOUNDS\n"
        "When you create a useful vocal chain, save it as a preset so you "
        "do not have to rebuild it from scratch.\n"
        "\n"
        "\n"
        "TROUBLESHOOTING\n"
        "\n"
        "VOCAL SOUNDS TOO PROCESSED\n"
        "Reduce VOCAL BOOST, DRIVE, COMPRESS or other intensive processing. "
        "You can also reduce the overall effect using MIX controls.\n"
        "\n"
        "VOCAL SOUNDS TOO BRIGHT\n"
        "Reduce AIR or other high-frequency enhancement and check DE-ESS.\n"
        "\n"
        "VOCAL SOUNDS TOO DARK\n"
        "Try increasing AIR carefully or reducing excessive damping in "
        "the Space section.\n"
        "\n"
        "VOCAL FEELS TOO WIDE\n"
        "Reduce DOUBLER WIDTH or the amount of additional stereo processing.\n"
        "\n"
        "CPU USAGE IS HIGH\n"
        "Try reducing oversampling, processing quality or switching CPU "
        "MODE to a lower-CPU setting.\n"
        "\n"
        "\n"
        "FINAL TIP\n"
        "\n"
        "OFFOR Vocal Pro is designed to give you control, not force you "
        "into one sound.\n"
        "\n"
        "Start with the natural vocal. Build the processing gradually. "
        "Use your ears, compare often and stop processing when the vocal "
        "already sounds right.\n"
        "\n"
        "\n"
        "OFFOR VOCAL PRO\n"
        "ONNTECH\n"
        "OFFOR AUDIO\n"
        "\n"
        "Thank you for using OFFOR Vocal Pro.";
}

//==============================================================================
// RESET USER GUIDE SCROLL
//==============================================================================

void SettingsPanel::resetUserGuideScroll()
{
    userGuideScrollPosition = 0.0f;
    userGuideIdleCounter = 0;

    if (userGuideViewport.getViewedComponent() != nullptr)
    {
        userGuideViewport.setViewPosition(
            0,
            0);
    }
}


//==============================================================================
// TIMER CALLBACK
//==============================================================================
//
// User Guide automatic scrolling.
//
// Behaviour:
//
//     - Guide is on screen
//         -> slow automatic scrolling.
//
//     - Mouse is over guide
//         -> automatic scrolling stops.
//
//     - User moves mouse wheel
//         -> manual scrolling is handled by UserGuideViewport.
//
//     - Mouse leaves guide
//         -> automatic scrolling resumes after a short delay.
//
//     - Bottom is reached
//         -> scrolling returns to the top and continues.
//
//==============================================================================

void SettingsPanel::timerCallback()
{
    //==========================================================================
    // ONLY RUN FOR USER GUIDE
    //==========================================================================

    if (currentPage != Page::userGuide)
        return;

    if (!userGuideViewport.isVisible())
        return;


    //==========================================================================
    // MOUSE IS OVER GUIDE
    //==========================================================================

    //
    // While the mouse is inside the guide, do absolutely nothing.
    //
    // This prevents the automatic scrolling from fighting with the user's
    // mouse wheel or drag scrolling.
    //

    if (userGuideMouseOver)
    {
        userGuideIdleCounter = 0;
        return;
    }


    //==========================================================================
    // WAIT BEFORE RESUMING
    //==========================================================================

    //
    // Timer runs at 30 Hz.
    //
    // 45 ticks / 30 Hz = 1.5 seconds.
    //
    constexpr int resumeDelayTicks = 45;

    if (userGuideIdleCounter < resumeDelayTicks)
    {
        ++userGuideIdleCounter;
        return;
    }


    //==========================================================================
    // GET GUIDE CONTENT
    //==========================================================================

    const auto* content =
        userGuideViewport.getViewedComponent();

    if (content == nullptr)
        return;


    //==========================================================================
    // CALCULATE MAXIMUM SCROLL
    //==========================================================================

    const int viewportHeight =
        userGuideViewport.getHeight();

    const int contentHeight =
        content->getHeight();

    const int maxScroll =
        juce::jmax(
            0,
            contentHeight
                - viewportHeight);

    if (maxScroll <= 0)
        return;


    //==========================================================================
    // GET CURRENT POSITION
    //==========================================================================

    const int actualPosition =
        userGuideViewport.getViewPositionY();


    userGuideScrollPosition =
        static_cast<float>(
            actualPosition);


    //==========================================================================
    // AUTOMATIC SCROLL SPEED
    //==========================================================================

    //
    // Timer = 30 Hz
    //
    // 0.35 pixels per tick
    // = approximately 10.5 pixels per second.
    //
    // This gives a slow, readable movement.
    //

    constexpr float automaticScrollSpeed =
        0.35f;


    userGuideScrollPosition +=
        automaticScrollSpeed;


    //==========================================================================
    // LOOP BACK TO TOP
    //==========================================================================

    if (userGuideScrollPosition >=
        static_cast<float>(maxScroll))
    {
        userGuideScrollPosition = 0.0f;
    }


    //==========================================================================
    // APPLY SCROLL
    //==========================================================================

    userGuideViewport.setViewPosition(
        0,
        static_cast<int>(
            userGuideScrollPosition));
}


//==============================================================================
// SELECT PAGE
//==============================================================================

void SettingsPanel::selectPage(
    Page page)
{
    currentPage =
        page;

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

    userGuideTab.setSelected(
        page == Page::userGuide);

    feedbackTab.setSelected(
        page == Page::feedback);

    //==========================================================================
    // Whenever the User Guide is opened, start it from the beginning.
    //==========================================================================

    if (page == Page::userGuide)
    {
        resetUserGuideScroll();

        userGuideIdleCounter = 0;
    }

    updatePageVisibility();

    resized();

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

    const bool userGuide =
        currentPage == Page::userGuide;

    const bool feedback =
        currentPage == Page::feedback;

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
    customizeColoursButton.setVisible(general);

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

    // ============================================================
    // USER GUIDE VISIBILITY
    // ============================================================

    userGuideTitle.setVisible(
        userGuide);

    userGuideDescription.setVisible(
        userGuide);

    userGuideViewport.setVisible(
        userGuide);


    //==========================================================================
    // FEEDBACK
    //==========================================================================

    if (feedbackPanel != nullptr)
        feedbackPanel->setVisible(
            feedback);
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
// CONFIGURE LABEL
//==============================================================================
//
// Central helper used throughout SettingsPanel.
//
// Keeps label configuration consistent:
//
//      - Text
//      - Font size
//      - Text colour
//      - Justification
//      - Transparent background
//      - No outline
//
//==============================================================================

void SettingsPanel::configureLabel(
    juce::Label& label,
    const juce::String& text,
    float fontSize,
    juce::Colour colour,
    juce::Justification justification)
{
    label.setText(
        text,
        juce::dontSendNotification);

    label.setFont(
        juce::Font(fontSize));

    label.setColour(
        juce::Label::textColourId,
        colour);

    label.setJustificationType(
        justification);

    label.setColour(
        juce::Label::backgroundColourId,
        juce::Colours::transparentBlack);

    label.setColour(
        juce::Label::outlineColourId,
        juce::Colours::transparentBlack);
}


//==============================================================================
// DRAW SECTION LINE
//==============================================================================

void SettingsPanel::drawSectionLine(
    juce::Graphics& g,
    int y)
{
    const auto colours =
        ThemeManager::get().getColours();

    g.setColour(
        colours.border);

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
    const auto colours =
        ThemeManager::get().getColours();

    //==========================================================================
    // BACKGROUND
    //==========================================================================

    g.fillAll(
        colours.background);

    auto bounds =
        getLocalBounds().toFloat();

    auto panel =
        bounds.reduced(1.0f);

    g.setColour(
        colours.panel);

    g.fillRoundedRectangle(
        panel,
        12.0f);

    g.setColour(
        colours.border);

    g.drawRoundedRectangle(
        panel.reduced(0.5f),
        12.0f,
        1.0f);

    //==========================================================================
    // HEADER LINE
    //==========================================================================

    g.setColour(
        colours.border);

    g.drawHorizontalLine(
        70,
        20.0f,
        static_cast<float>(
            getWidth() - 20));

    //==========================================================================
    // SETTINGS TITLE
    //==========================================================================

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(16.0f)
                .withStyle("Bold")));

    g.setColour(
        colours.text);

    g.drawText(
        "SETTINGS",
        28,
        16,
        180,
        22,
        juce::Justification::centredLeft,
        false);

    //==========================================================================
    // PRODUCT
    //==========================================================================

    g.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(9.0f)
                .withStyle("Medium")));

    g.setColour(
        colours.muted);

    g.drawText(
        "OFFOR VOCAL PRO",
        29,
        39,
        180,
        16,
        juce::Justification::centredLeft,
        false);

    //==========================================================================
    // NAVIGATION DIVIDER
    //==========================================================================

    g.setColour(
        colours.border);

    g.drawHorizontalLine(
        115,
        20.0f,
        static_cast<float>(
            getWidth() - 20));

    //==========================================================================
    // CONTENT PANEL
    //==========================================================================

    g.setColour(
        colours.panel.withAlpha(0.55f));

    g.fillRoundedRectangle(
        20.0f,
        126.0f,
        static_cast<float>(
            getWidth() - 40),
        static_cast<float>(
            getHeight() - 146),
        8.0f);

    g.setColour(
        colours.border.withAlpha(0.45f));

    g.drawRoundedRectangle(
        20.0f,
        126.0f,
        static_cast<float>(
            getWidth() - 40),
        static_cast<float>(
            getHeight() - 146),
        8.0f,
        1.0f);

    //==========================================================================
    // PAGE DIVIDERS
    //==========================================================================

    if (currentPage == Page::general)
    {
        g.setColour(
            colours.border);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(
                getWidth() - 40));

        g.drawHorizontalLine(
            365,
            40.0f,
            static_cast<float>(
                getWidth() - 40));
    }

    if (currentPage == Page::audio)
    {
        g.setColour(
            colours.border);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(
                getWidth() - 40));

        g.drawHorizontalLine(
            365,
            40.0f,
            static_cast<float>(
                getWidth() - 40));
    }

    if (currentPage == Page::display)
    {
        g.setColour(
            colours.border);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(
                getWidth() - 40));

        g.drawHorizontalLine(
            365,
            40.0f,
            static_cast<float>(
                getWidth() - 40));
    }

    if (currentPage == Page::performance)
    {
        g.setColour(
            colours.border);

        g.drawHorizontalLine(
            250,
            40.0f,
            static_cast<float>(
                getWidth() - 40));
    }

    if (currentPage == Page::about)
    {
        g.setColour(
            colours.border);

        g.drawHorizontalLine(
            360,
            70.0f,
            static_cast<float>(
                getWidth() - 70));
    }

    //==========================================================================
    // USER GUIDE
    //==========================================================================
    //
    // The guide itself is a child component, so there is intentionally no
    // additional text painted here.
    //
    //==========================================================================

    if (currentPage == Page::userGuide)
    {
        g.setColour(
            colours.border.withAlpha(0.35f));

        g.drawRoundedRectangle(
            30.0f,
            142.0f,
            static_cast<float>(
                getWidth() - 60),
            static_cast<float>(
                getHeight() - 160),
            7.0f,
            1.0f);
    }

    //==========================================================================
    // FEEDBACK
    //==========================================================================
    //
    // FeedbackPanel paints its own controls.
    // We only draw the surrounding page border here.
    //
    //==========================================================================

    if (currentPage == Page::feedback)
    {
        g.setColour(
            colours.border.withAlpha(0.35f));

        g.drawRoundedRectangle(
            30.0f,
            142.0f,
            static_cast<float>(
                getWidth() - 60),
            static_cast<float>(
                getHeight() - 160),
            7.0f,
            1.0f);
    }
}

//==============================================================================
// RESIZED
//==============================================================================

void SettingsPanel::resized()
{
    const int width =
        getWidth();

    const int height =
        getHeight();

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
    //==========================================================================
    //
    // Seven tabs:
    //
    // GENERAL
    // AUDIO
    // DISPLAY
    // PERFORMANCE
    // ABOUT
    // USER GUIDE
    // FEEDBACK
    //
    // All seven tabs receive exactly the same width.
    // This prevents overlapping or uneven tab areas.
    //
    //==========================================================================

    const int navigationY =
        77;

    const int navigationHeight =
        35;

    const int navigationLeft =
        30;

    const int navigationRight =
        width - 30;

    const int navigationWidth =
        navigationRight -
        navigationLeft;

    const int numberOfTabs =
        7;

    const int tabWidth =
        navigationWidth / numberOfTabs;

    //--------------------------------------------------------------------------
    // GENERAL
    //--------------------------------------------------------------------------

    generalTab.setBounds(
        navigationLeft + tabWidth * 0,
        navigationY,
        tabWidth,
        navigationHeight);

    //--------------------------------------------------------------------------
    // AUDIO
    //--------------------------------------------------------------------------

    audioTab.setBounds(
        navigationLeft + tabWidth * 1,
        navigationY,
        tabWidth,
        navigationHeight);

    //--------------------------------------------------------------------------
    // DISPLAY
    //--------------------------------------------------------------------------

    displayTab.setBounds(
        navigationLeft + tabWidth * 2,
        navigationY,
        tabWidth,
        navigationHeight);

    //--------------------------------------------------------------------------
    // PERFORMANCE
    //--------------------------------------------------------------------------

    performanceTab.setBounds(
        navigationLeft + tabWidth * 3,
        navigationY,
        tabWidth,
        navigationHeight);

    //--------------------------------------------------------------------------
    // ABOUT
    //--------------------------------------------------------------------------

    aboutTab.setBounds(
        navigationLeft + tabWidth * 4,
        navigationY,
        tabWidth,
        navigationHeight);

    //--------------------------------------------------------------------------
    // USER GUIDE
    //--------------------------------------------------------------------------

    userGuideTab.setBounds(
        navigationLeft + tabWidth * 5,
        navigationY,
        tabWidth,
        navigationHeight);

    //--------------------------------------------------------------------------
    // FEEDBACK
    //--------------------------------------------------------------------------
    //
    // Give Feedback now occupies exactly the final seventh slot.
    //
    //--------------------------------------------------------------------------

    feedbackTab.setBounds(
        navigationLeft + tabWidth * 6,
        navigationY,

        // Use the remaining pixels so there is no 1–6 pixel gap
        // caused by integer division.
        navigationRight -
            (navigationLeft + tabWidth * 6),

        navigationHeight);

    //==========================================================================
    // PAGE CONTENT
    //==========================================================================

    const int left =
        50;

    const int right =
        width - 50;

    const int selectorWidth =
        150;

    const int selectorHeight =
        34;

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

    customizeColoursButton.setBounds(
        right - selectorWidth,
        478,
        selectorWidth,
        34);

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

    aboutTitle.setBounds(
        left,
        145,
        300,
        30);

    aboutDescription.setBounds(
        left,
        176,
        right - left,
        22);

    productNameLabel.setBounds(
        left,
        215,
        right - left,
        35);

    versionLabel.setBounds(
        left,
        253,
        right - left,
        20);

    companyLabel.setBounds(
        left,
        285,
        right - left,
        24);

    descriptionLabel.setBounds(
        left,
        315,
        right - left,
        24);

    licenseTitleLabel.setBounds(
        left,
        380,
        250,
        20);

    licenseStatusLabel.setBounds(
        left,
        410,
        250,
        25);

    websiteButton.setBounds(
        left,
        460,
        155,
        38);

    supportButton.setBounds(
        left + 170,
        460,
        120,
        38);


    // ============================================================
    // USER GUIDE
    // ============================================================

    userGuideTitle.setBounds(
        left,
        145,
        300,
        30);

    userGuideDescription.setBounds(
        left,
        176,
        right - left,
        22);

    // ------------------------------------------------------------
    // GUIDE VIEWPORT
    // ------------------------------------------------------------

    userGuideViewport.setBounds(
        32,
        205,
        width - 64,
        height - 225);

    // ------------------------------------------------------------
    // UPDATE CONTENT SIZE
    // ------------------------------------------------------------

    updateUserGuideLayout();



    //==========================================================================
    // FEEDBACK
    //==========================================================================
    //
    // FeedbackPanel handles its own internal controls.
    // SettingsPanel only gives it the available page area.
    //
    //==========================================================================

    if (feedbackPanel != nullptr)
    {
        feedbackPanel->setBounds(
            left,
            145,
            right - left,
            height - 175);
    }




    //==========================================================================
    // CUSTOM COLOUR PANEL
    //==========================================================================

    if (themeColorPanel != nullptr)
    {
        themeColorPanel->setBounds(
            getLocalBounds().reduced(25));
    }

    juce::ignoreUnused(height);
}