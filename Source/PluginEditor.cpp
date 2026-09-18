#include "PluginEditor.h"
#include "BinaryData.h"
#include <iostream>

#include <cmath>


//==============================================================================
// PRESET NAMES
//==============================================================================
//
// These are the initial factory presets.
// The preset system stores the complete APVTS state,
// so adding more parameters later will automatically
// make them part of preset/A-B state storage.
//

namespace
{
    const juce::StringArray factoryPresetNames =
    {
        "Default",
        "Clean Vocal",
        "Bright Vocal",
        "Warm Vocal",
        "Radio Vocal",
        "Wide Vocal"
    };
}

//==============================================================================
// COLOURS
//==============================================================================

const juce::Colour
OfforVocalProAudioProcessorEditor::backgroundColour =
    juce::Colour::fromString("FF0A0B0D");

const juce::Colour
OfforVocalProAudioProcessorEditor::panelColour =
    juce::Colour::fromString("FF131518");

const juce::Colour
OfforVocalProAudioProcessorEditor::panelColour2 =
    juce::Colour::fromString("FF191C20");

const juce::Colour
OfforVocalProAudioProcessorEditor::borderColour =
    juce::Colour::fromString("FF292D32");

const juce::Colour
OfforVocalProAudioProcessorEditor::textColour =
    juce::Colour::fromString("FFF2F3F4");

const juce::Colour
OfforVocalProAudioProcessorEditor::mutedColour =
    juce::Colour::fromString("FF777D85");

const juce::Colour
OfforVocalProAudioProcessorEditor::accentColour =
    juce::Colour::fromString("FFD72F35");

const juce::Colour
OfforVocalProAudioProcessorEditor::accentDarkColour =
    juce::Colour::fromString("FF7E1C21");

const juce::Colour
OfforVocalProAudioProcessorEditor::displayColour =
    juce::Colour::fromString("FF08090B");

//==============================================================================
// MODULE BUTTON
//==============================================================================

OfforVocalProAudioProcessorEditor::ModuleButton::ModuleButton()
{
    setWantsKeyboardFocus(false);
    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);
}


void
OfforVocalProAudioProcessorEditor::ModuleButton::setIcon(
    const juce::Image& image)
{
    if (!image.isValid())
        return;

    // ICON COLOR
    // Change this color to change ALL module icons.
    // Examples:
    // White  = juce::Colours::white
    // Red    = juce::Colours::red
    // Purple = juce::Colour(0xffB45CFF)
    // Orange = juce::Colour(0xffff8a3d)
    // ==========================================================

   
    const juce::Colour iconColour =
        juce::Colours::white;

    // ==========================================================
    // CREATE TRANSPARENT ARGB IMAGE
    // ==========================================================

    icon =
        juce::Image(
            juce::Image::ARGB,
            image.getWidth(),
            image.getHeight(),
            true);

    // Make absolutely sure the entire new image starts transparent.
    icon.clear(
        icon.getBounds(),
        juce::Colours::transparentBlack);

    // ==========================================================
    // COPY ONLY THE ORIGINAL ALPHA
    // ==========================================================

    juce::Image::BitmapData sourceData(
        image,
        juce::Image::BitmapData::readOnly);

    juce::Image::BitmapData destinationData(
        icon,
        juce::Image::BitmapData::readWrite);

    for (int y = 0; y < image.getHeight(); ++y)
    {
        for (int x = 0; x < image.getWidth(); ++x)
        {
            const auto sourcePixel =
                sourceData.getPixelColour(x, y);

            const float alpha =
                sourcePixel.getAlpha();

            destinationData.setPixelColour(
                x,
                y,
                iconColour.withAlpha(alpha));
        }
    }

    repaint();
}


void
OfforVocalProAudioProcessorEditor::ModuleButton::setModuleName(
    const juce::String& name)
{
    moduleName = name;
    repaint();
}

void
OfforVocalProAudioProcessorEditor::ModuleButton::setSelected(
    bool shouldBeSelected)
{
    selected = shouldBeSelected;
    repaint();
}

void
OfforVocalProAudioProcessorEditor::ModuleButton::paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat();

    auto bg =
        selected
            ? juce::Colour::fromString("FF24272B")
            : panelColour;

    if (shouldDrawButtonAsHighlighted)
        bg = bg.brighter(0.08f);

    if (shouldDrawButtonAsDown)
        bg = bg.brighter(0.14f);

    g.setColour(bg);

    g.fillRoundedRectangle(
        bounds,
        9.0f);

    if (selected)
    {
        g.setColour(accentColour);

        g.fillRoundedRectangle(
            juce::Rectangle<float>(
                0.0f,
                7.0f,
                3.0f,
                bounds.getHeight() - 14.0f),
            1.5f);
    }

    g.setColour(
        selected
            ? accentColour
            : borderColour);

    g.drawRoundedRectangle(
        bounds.reduced(0.5f),
        9.0f,
        1.0f);

    const int iconSize =
        juce::jmin(
            28,
            getHeight() - 18);

    auto iconBounds =
        juce::Rectangle<int>(
            14,
            (getHeight() - iconSize) / 2,
            iconSize,
            iconSize);

    if (icon.isValid())
    {
        g.setOpacity(
            selected ? 1.0f : 0.72f);

        g.drawImageWithin(
            icon,
            iconBounds.getX(),
            iconBounds.getY(),
            iconBounds.getWidth(),
            iconBounds.getHeight(),
            juce::RectanglePlacement::centred,
            false);

        g.setOpacity(1.0f);
    }

    g.setColour(
        selected
            ? textColour
            : mutedColour);

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(10.5f)
                .withStyle("Bold")));

    g.drawText(
        moduleName,
        iconBounds.getRight() + 11,
        0,
        getWidth() - iconBounds.getRight() - 18,
        getHeight(),
        juce::Justification::centredLeft);
}


//==============================================================================
// LICENSE ACTION BUTTON
//==============================================================================
//
// Custom-painted licensing buttons.
//
// We deliberately do NOT use the default JUCE TextButton appearance.
// These buttons are part of the commercial product's licensing screen,
// so they use the same dark/red-orange visual language as OFFOR Vocal Pro.
//
//==============================================================================

OfforVocalProAudioProcessorEditor::LicenseActionButton::
LicenseActionButton(
    const juce::String& buttonText)
    : juce::Button(buttonText),
      text(buttonText)
{
    setClickingTogglesState(false);

    setWantsKeyboardFocus(false);

    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::LicenseActionButton::
paintButton(
    juce::Graphics& g,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds =
        getLocalBounds()
            .toFloat()
            .reduced(1.0f);

    // ----------------------------------------------------------
    // Button colours
    // ----------------------------------------------------------

    const auto normal =
        juce::Colour(0xffb82127);

    const auto hover =
        juce::Colour(0xffd72f35);

    const auto pressed =
        juce::Colour(0xff8e171c);

    const auto border =
        juce::Colour(0xffff5a36);

    const auto currentColour =
        shouldDrawButtonAsDown
            ? pressed
            : shouldDrawButtonAsHighlighted
                ? hover
                : normal;

    // ----------------------------------------------------------
    // Shadow
    // ----------------------------------------------------------

    g.setColour(
        juce::Colours::black.withAlpha(0.65f));

    g.fillRoundedRectangle(
        bounds.translated(0.0f, 3.0f),
        7.0f);

    // ----------------------------------------------------------
    // Main body
    // ----------------------------------------------------------

    g.setColour(currentColour);

    g.fillRoundedRectangle(
        bounds,
        7.0f);

    // ----------------------------------------------------------
    // Border
    // ----------------------------------------------------------

    g.setColour(
        border.withAlpha(
            shouldDrawButtonAsHighlighted
                ? 0.9f
                : 0.55f));

    g.drawRoundedRectangle(
        bounds,
        7.0f,
        1.0f);

    // ----------------------------------------------------------
    // Text
    // ----------------------------------------------------------

    g.setColour(
        juce::Colours::white);

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(10.0f)
                .withStyle("Bold")));

    g.drawText(
        text,
        getLocalBounds(),
        juce::Justification::centred);
}


//==============================================================================
// LICENSE OVERLAY
//==============================================================================

OfforVocalProAudioProcessorEditor::LicenseOverlay::
LicenseOverlay(
    OfforVocalProAudioProcessorEditor& editor)
    : owner(editor),
      activateButton("ACTIVATE LICENSE"),
      buyButton("BUY LICENSE")
{
    setOpaque(false);

    // ----------------------------------------------------------
    // TITLE
    // ----------------------------------------------------------

    titleLabel.setText(
        "TRIAL COMPLETE",
        juce::dontSendNotification);

    titleLabel.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(22.0f)
                .withStyle("Bold")));

    titleLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xfff2f3f4));

    titleLabel.setJustificationType(
        juce::Justification::centred);

    addAndMakeVisible(titleLabel);

    // ----------------------------------------------------------
    // MAIN MESSAGE
    // ----------------------------------------------------------

    messageLabel.setText(
        "You've used all 10 free sessions.",
        juce::dontSendNotification);

    messageLabel.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(12.0f)
                .withStyle("Bold")));

    messageLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xffd7d9dc));

    messageLabel.setJustificationType(
        juce::Justification::centred);

    addAndMakeVisible(messageLabel);

    // ----------------------------------------------------------
    // INSTRUCTION
    // ----------------------------------------------------------

    instructionLabel.setText(
        "Activate your license to continue using Offor Vocal Pro.",
        juce::dontSendNotification);

    instructionLabel.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(10.0f)));

    instructionLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xff777d85));

    instructionLabel.setJustificationType(
        juce::Justification::centred);

    addAndMakeVisible(instructionLabel);

    // ----------------------------------------------------------
    // LICENSE FIELD
    // ----------------------------------------------------------

    licenseEditor.setTextToShowWhenEmpty(
        "Enter your OFFOR license key",
        juce::Colour(0xff666b72));

    licenseEditor.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(11.0f)));

    licenseEditor.setColour(
        juce::TextEditor::backgroundColourId,
        juce::Colour(0xff08090b));

    licenseEditor.setColour(
        juce::TextEditor::textColourId,
        juce::Colour(0xfff2f3f4));

    licenseEditor.setColour(
        juce::TextEditor::outlineColourId,
        juce::Colour(0xff292d32));

    licenseEditor.setColour(
        juce::TextEditor::focusedOutlineColourId,
        juce::Colour(0xffd72f35));

    licenseEditor.setColour(
        juce::TextEditor::highlightColourId,
        juce::Colour(0xff7e1c21));

    licenseEditor.setBorder(
        juce::BorderSize<int>(1));

    licenseEditor.setJustification(
        juce::Justification::centredLeft);

    licenseEditor.setSelectAllWhenFocused(true);

    addAndMakeVisible(licenseEditor);

    // ----------------------------------------------------------
    // STATUS
    // ----------------------------------------------------------

    statusLabel.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(9.0f)
                .withStyle("Bold")));

    statusLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xff777d85));

    statusLabel.setJustificationType(
        juce::Justification::centred);

    statusLabel.setVisible(false);

    addAndMakeVisible(statusLabel);

    // ----------------------------------------------------------
    // BUTTONS
    // ----------------------------------------------------------

    addAndMakeVisible(
        activateButton);

    addAndMakeVisible(
        buyButton);

    activateButton.onClick =
        [this]
        {
            owner.beginLicenseActivation();
        };

    buyButton.onClick =
        [this]
        {
            owner.openLicensePurchasePage();
        };
}


//==============================================================================

OfforVocalProAudioProcessorEditor::LicenseOverlay::
~LicenseOverlay()
{
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::LicenseOverlay::
paint(
    juce::Graphics& g)
{
    auto area =
        getLocalBounds().toFloat();

    // ----------------------------------------------------------
    // Darkened background
    // ----------------------------------------------------------

    g.setColour(
        juce::Colours::black.withAlpha(0.78f));

    g.fillRect(area);

    // ----------------------------------------------------------
    // Main licensing card
    // ----------------------------------------------------------

    auto card =
        area.reduced(
            juce::jmin(
                area.getWidth() * 0.12f,
                area.getHeight() * 0.13f));

    g.setColour(
        juce::Colour(0xff101216));

    g.fillRoundedRectangle(
        card,
        14.0f);

    // ----------------------------------------------------------
    // Card border
    // ----------------------------------------------------------

    g.setColour(
        juce::Colour(0xff34383e));

    g.drawRoundedRectangle(
        card,
        14.0f,
        1.0f);

    // ----------------------------------------------------------
    // Accent line
    // ----------------------------------------------------------

    auto accentLine =
        card.withX(
                card.getX() + 30.0f)
            .withWidth(
                card.getWidth() - 60.0f)
            .withHeight(3.0f)
            .withY(
                card.getY() + 22.0f);

    g.setColour(
        juce::Colour(0xffd72f35));

    g.fillRoundedRectangle(
        accentLine,
        1.5f);

    // ----------------------------------------------------------
    // Small lock / license indicator
    // ----------------------------------------------------------

    const float iconSize = 34.0f;

    auto iconBounds =
        juce::Rectangle<float>(
            card.getCentreX() - iconSize * 0.5f,
            card.getY() + 45.0f,
            iconSize,
            iconSize);

    g.setColour(
        juce::Colour(0xff191c20));

    g.fillEllipse(iconBounds);

    g.setColour(
        juce::Colour(0xffff5a36));

    g.drawEllipse(
        iconBounds,
        1.2f);

    // Simple keyhole symbol.
    g.fillEllipse(
        iconBounds.getCentreX() - 3.0f,
        iconBounds.getCentreY() - 5.0f,
        6.0f,
        6.0f);

    g.fillRoundedRectangle(
        iconBounds.getCentreX() - 2.0f,
        iconBounds.getCentreY(),
        4.0f,
        8.0f,
        1.5f);
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::LicenseOverlay::
resized()
{
    auto area =
        getLocalBounds();

    const int cardWidth =
        juce::jmin(
            520,
            area.getWidth() - 80);

    const int cardHeight =
        juce::jmin(
            350,
            area.getHeight() - 50);

    const int x =
        area.getCentreX()
        - cardWidth / 2;

    const int y =
        area.getCentreY()
        - cardHeight / 2;

    auto card =
        juce::Rectangle<int>(
            x,
            y,
            cardWidth,
            cardHeight);

    titleLabel.setBounds(
        card.getX() + 30,
        card.getY() + 88,
        card.getWidth() - 60,
        32);

    messageLabel.setBounds(
        card.getX() + 30,
        card.getY() + 130,
        card.getWidth() - 60,
        24);

    instructionLabel.setBounds(
        card.getX() + 30,
        card.getY() + 157,
        card.getWidth() - 60,
        24);

    licenseEditor.setBounds(
        card.getX() + 55,
        card.getY() + 200,
        card.getWidth() - 110,
        38);

    activateButton.setBounds(
        card.getCentreX() - 155,
        card.getY() + 253,
        145,
        40);

    buyButton.setBounds(
        card.getCentreX() + 10,
        card.getY() + 253,
        145,
        40);

    statusLabel.setBounds(
        card.getX() + 35,
        card.getY() + 300,
        card.getWidth() - 70,
        22);
}


//====================================================
//==========================

void
OfforVocalProAudioProcessorEditor::LicenseOverlay::
setActivationState(
    bool newActivating,
    const juce::String& message)
{
    activating = newActivating;

    activateButton.setEnabled(
        !activating);

    buyButton.setEnabled(
        !activating);

    licenseEditor.setEnabled(
        !activating);

    if (activating)
    {
        statusLabel.setVisible(true);

        statusLabel.setColour(
            juce::Label::textColourId,
            juce::Colour(0xffff9a55));

        statusLabel.setText(
            "ACTIVATING LICENSE...",
            juce::dontSendNotification);
    }
    else if (message.isNotEmpty())
    {
        statusLabel.setVisible(true);

        statusLabel.setColour(
            juce::Label::textColourId,
            juce::Colour(0xffff5a55));

        statusLabel.setText(
            message,
            juce::dontSendNotification);
    }
    else
    {
        statusLabel.setVisible(false);
    }

    repaint();
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::LicenseOverlay::
clearActivationMessage()
{
    statusLabel.setVisible(false);

    repaint();
}


//==============================================================================

juce::String
OfforVocalProAudioProcessorEditor::LicenseOverlay::
getLicenseKey() const
{
    return licenseEditor.getText()
        .trim();
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::LicenseOverlay::
setLicenseKey(
    const juce::String& key)
{
    licenseEditor.setText(
        key,
        false);
}

// ==========================================================
// LICENSE ACTIVATION THREAD
// ==========================================================

OfforVocalProAudioProcessorEditor::LicenseActivationThread::LicenseActivationThread(
    OfforVocalProAudioProcessorEditor& editor,
    const juce::String& key)
    : juce::Thread("OFFOR License Activation"),
      owner(editor),
      licenseKey(key)
{
}

void OfforVocalProAudioProcessorEditor::LicenseActivationThread::run()
{
    const bool success =
        owner.audioProcessor.activateLicense(licenseKey);

    juce::MessageManager::callAsync(
        [safeOwner = juce::Component::SafePointer<
            OfforVocalProAudioProcessorEditor>(&owner),
         success]()
        {
            if (safeOwner != nullptr)
            {
                safeOwner->finishLicenseActivation(
                    success,
                    success
                        ? "License activated successfully."
                        : "License activation failed. Please check your license key."
                );
            }
        });
}

//==============================================================================
// CONSTRUCTOR
//==============================================================================

OfforVocalProAudioProcessorEditor::
OfforVocalProAudioProcessorEditor(
    OfforVocalProAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(
        920,
        570);

    setResizable(
        true,
        true);

    setResizeLimits(
        820,
        500,
        1400,
        900);

    loadImages();

    // ==========================================================
    // HEADER
    // ==========================================================

    setupTitle(
        titleLabel,
        "OFFOR VOCAL PRO");

    titleLabel.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(18.0f)
                .withStyle("Bold")));

    addAndMakeVisible(
        titleLabel);

    setupLabel(
        subtitleLabel,
        "VOCAL PRODUCTION");

    subtitleLabel.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(8.5f)
                .withStyle("Bold")));

    addAndMakeVisible(
        subtitleLabel);

    setupLabel(
        versionLabel,
        "V1.0.0");

    versionLabel.setJustificationType(
        juce::Justification::centredRight);

    addAndMakeVisible(
        versionLabel);

    // ==========================================================
    // SETTINGS BUTTON
    // ==========================================================
    //
    // ImageButton is used instead of TextButton because JUCE
    // ImageButton supports normal / hover / pressed images.
    //
    // The image itself is converted to white inside loadImages().
    // ==========================================================

    settingsButton.setWantsKeyboardFocus(false);

    settingsButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    // ----------------------------------------------------------
    // Use the white settings icon.
    // ----------------------------------------------------------

    if (settingsIcon.isValid())
    {
        settingsButton.setImages(
            false,
            true,
            true,
            settingsIcon,
            1.0f,
            juce::Colours::white,
            settingsIcon,
            1.0f,
            juce::Colours::white,
            settingsIcon,
            1.0f,
            juce::Colours::white,
            0.0f);
    }

    // ==========================================================
    // SETTINGS PANEL
    // ==========================================================

    // Create the separate settings screen.
    settingsPanel = std::make_unique<SettingsPanel>();

    // Add it above the main plugin UI.
    addAndMakeVisible(*settingsPanel);

    // Start hidden. The Settings icon will open it.
    settingsPanel->setVisible(false);

    // Settings panel close button.
    settingsPanel->onClose = [this]()
    {
        hideSettingsPanel();
    };

    // Existing Settings icon opens/closes the panel.
    settingsButton.onClick = [this]()
    {
        if (settingsPanel == nullptr)
            return;

        if (settingsPanel->isVisible())
            hideSettingsPanel();
        else
            showSettingsPanel();
    };

    // ----------------------------------------------------------
    // SETTINGS action.
    // ----------------------------------------------------------

    // settingsButton.onClick =
    //     [this]
    //     {
    //         juce::AlertWindow::showMessageBoxAsync(
    //             juce::AlertWindow::InfoIcon,
    //             "OFFOR Vocal Pro",
    //             "Settings panel will be available here.",
    //             "OK");
    //     };

    addAndMakeVisible(settingsButton);


    // ==========================================================
    // SETTINGS ICON
    // ==========================================================
    //
    // The button itself remains transparent.
    // The actual white settings icon is supplied separately.
    //


    // ==========================================================
    // BYPASS
    // ==========================================================

    bypassButton.setButtonText(
        "BYPASS");

    bypassButton.setClickingTogglesState(
        true);

    bypassButton.setColour(
        juce::ToggleButton::textColourId,
        mutedColour);

    bypassButton.setColour(
        juce::ToggleButton::tickColourId,
        accentColour);

    addAndMakeVisible(
        bypassButton);

    // ==========================================================
    // MODULE BUTTONS
    // ==========================================================

    setupModuleButton(
        tunerButton,
        radioTunerIcon,
        "TUNER");

    setupModuleButton(
        doublerButton,
        doublerIcon,
        "DOUBLER");

    setupModuleButton(
        harmonyButton,
        harmonyIcon,
        "HARMONY");

    setupModuleButton(
        fxButton,
        fxIcon,
        "CREATIVE FX");

    setupModuleButton(
        spaceButton,
        spaceIcon,
        "SPACE");

    tunerButton.onClick =
        [this]
        {
            selectModule(Module::tuner);
        };

    doublerButton.onClick =
        [this]
        {
            selectModule(Module::doubler);
        };

    harmonyButton.onClick =
        [this]
        {
            selectModule(Module::harmony);
        };

    fxButton.onClick =
        [this]
        {
            selectModule(Module::creativeFx);
        };

    spaceButton.onClick =
        [this]
        {
            selectModule(Module::space);
        };

    // ==========================================================
    // GLOBAL CONTROLS
    // ==========================================================

    setupGlobalSlider(
        inputSlider);

    setupGlobalSlider(
        globalMixSlider);

    setupGlobalSlider(
        outputSlider);

    setupLabel(
        inputLabel,
        "INPUT");

    setupLabel(
        globalMixLabel,
        "MIX");

    setupLabel(
        outputLabel,
        "OUTPUT");

    addAndMakeVisible(
        inputSlider);

    addAndMakeVisible(
        globalMixSlider);

    addAndMakeVisible(
        outputSlider);

    addAndMakeVisible(
        inputLabel);

    addAndMakeVisible(
        globalMixLabel);

    addAndMakeVisible(
        outputLabel);

    // ==========================================================
    // LEVEL METERS
    // ==========================================================
    //
    // The meters are intentionally separate from the INPUT,
    // MIX and OUTPUT hardware knobs.
    //

    inputMeter.setLabel("INPUT");
    outputMeter.setLabel("OUTPUT");

    inputMeter.setMinimumDecibels(-60.0f);
    inputMeter.setMaximumDecibels(0.0f);

    outputMeter.setMinimumDecibels(-60.0f);
    outputMeter.setMaximumDecibels(0.0f);

    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    // ==========================================================
    // TUNER
    // ==========================================================


    setupLabel(
        tunerStatusLabel,
        "READY");

    addAndMakeVisible(
        tunerTitleLabel);

    addAndMakeVisible(
        tunerStatusLabel);

    setupPitchDisplayLabel(
        detectedNoteLabel);

    detectedNoteLabel.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(58.0f)
                .withStyle("Bold")));

    detectedNoteLabel.setColour(
        juce::Label::textColourId,
        textColour);

    setupPitchDisplayLabel(
        detectedFrequencyLabel);

    setupPitchDisplayLabel(
        detectedCentsLabel);

    setupPitchDisplayLabel(
        detectedConfidenceLabel);

    addAndMakeVisible(
        detectedNoteLabel);

    addAndMakeVisible(
        detectedFrequencyLabel);

    addAndMakeVisible(
        detectedCentsLabel);

    addAndMakeVisible(
        detectedConfidenceLabel);

    setupLabel(
        keyLabel,
        "KEY");

    setupLabel(
        scaleLabel,
        "SCALE");

    setupLabel(
        modeLabel,
        "MODE");

    setupComboBox(
        keyComboBox);

    setupComboBox(
        scaleComboBox);

    setupComboBox(
        modeComboBox);

    addAndMakeVisible(
        keyLabel);

    addAndMakeVisible(
        scaleLabel);

    addAndMakeVisible(
        modeLabel);

    addAndMakeVisible(
        keyComboBox);

    addAndMakeVisible(
        scaleComboBox);

    addAndMakeVisible(
        modeComboBox);

    populateKeyComboBox();
    populateScaleComboBox();
    populateModeComboBox();

    setupSlider(
        tunerRetuneSlider);

    setupSlider(
        tunerSmoothSlider);

    setupSlider(
        tunerFormantSlider);

    setupSlider(
        tunerMixSlider);

    setupLabel(
        tunerRetuneLabel,
        "RETUNE");

    setupLabel(
        tunerSmoothLabel,
        "SMOOTH");

    setupLabel(
        tunerFormantLabel,
        "FORMANT");

    setupLabel(
        tunerMixLabel,
        "MIX");

    addAndMakeVisible(
        tunerRetuneSlider);

    addAndMakeVisible(
        tunerSmoothSlider);

    addAndMakeVisible(
        tunerFormantSlider);

    addAndMakeVisible(
        tunerMixSlider);

    addAndMakeVisible(
        tunerRetuneLabel);

    addAndMakeVisible(
        tunerSmoothLabel);

    addAndMakeVisible(
        tunerFormantLabel);

    addAndMakeVisible(
        tunerMixLabel);

    // ==========================================================
    // DOUBLER
    // ==========================================================


    addAndMakeVisible(
        doublerTitleLabel);

    setupSlider(
        doublerAmountSlider);

    setupSlider(
        doublerDetuneSlider);

    setupSlider(
        doublerTimingSlider);

    setupSlider(
        doublerWidthSlider);

    setupSlider(
        doublerMixSlider);

    setupLabel(
        doublerAmountLabel,
        "AMOUNT");

    setupLabel(
        doublerDetuneLabel,
        "DETUNE");

    setupLabel(
        doublerTimingLabel,
        "TIMING");

    setupLabel(
        doublerWidthLabel,
        "WIDTH");

    setupLabel(
        doublerMixLabel,
        "MIX");

    addAndMakeVisible(
        doublerAmountSlider);

    addAndMakeVisible(
        doublerDetuneSlider);

    addAndMakeVisible(
        doublerTimingSlider);

    addAndMakeVisible(
        doublerWidthSlider);

    addAndMakeVisible(
        doublerMixSlider);

    addAndMakeVisible(
        doublerAmountLabel);

    addAndMakeVisible(
        doublerDetuneLabel);

    addAndMakeVisible(
        doublerTimingLabel);

    addAndMakeVisible(
        doublerWidthLabel);

    addAndMakeVisible(
        doublerMixLabel);

    // ==========================================================
    // HARMONY
    // ==========================================================

    addAndMakeVisible(
        harmonyTitleLabel);

    setupLabel(
        harmonyVoice1Label,
        "VOICE 1");

    setupLabel(
        harmonyVoice2Label,
        "VOICE 2");

    setupLabel(
        harmonyVoice3Label,
        "VOICE 3");

    setupLabel(
        harmonyVoice4Label,
        "VOICE 4");

    setupComboBox(
        harmonyVoice1ComboBox);

    setupComboBox(
        harmonyVoice2ComboBox);

    setupComboBox(
        harmonyVoice3ComboBox);

    setupComboBox(
        harmonyVoice4ComboBox);

    addAndMakeVisible(
        harmonyVoice1Label);

    addAndMakeVisible(
        harmonyVoice2Label);

    addAndMakeVisible(
        harmonyVoice3Label);

    addAndMakeVisible(
        harmonyVoice4Label);

    addAndMakeVisible(
        harmonyVoice1ComboBox);

    addAndMakeVisible(
        harmonyVoice2ComboBox);

    addAndMakeVisible(
        harmonyVoice3ComboBox);

    addAndMakeVisible(
        harmonyVoice4ComboBox);

    populateHarmonyComboBoxes();

    setupSlider(
        harmonyMixSlider);

    setupLabel(
        harmonyMixLabel,
        "HARMONY MIX");

    addAndMakeVisible(
        harmonyMixSlider);

    addAndMakeVisible(
        harmonyMixLabel);

    // ==========================================================
    // CREATIVE FX
    // ==========================================================

    addAndMakeVisible(
        creativeFxTitleLabel);

    setupLabel(
        creativeFxTypeLabel,
        "EFFECT");

    setupLabel(
        creativeFxAmountLabel,
        "AMOUNT");

    setupLabel(
        creativeFxMixLabel,
        "MIX");

    setupComboBox(
        creativeFxTypeComboBox);

    setupSlider(
        creativeFxAmountSlider);

    setupSlider(
        creativeFxMixSlider);

    addAndMakeVisible(
        creativeFxTypeLabel);

    addAndMakeVisible(
        creativeFxAmountLabel);

    addAndMakeVisible(
        creativeFxMixLabel);

    addAndMakeVisible(
        creativeFxTypeComboBox);

    addAndMakeVisible(
        creativeFxAmountSlider);

    addAndMakeVisible(
        creativeFxMixSlider);

    populateCreativeFxComboBox();

    // ==========================================================
    // SPACE
    // ==========================================================

    addAndMakeVisible(
        spaceTitleLabel);

    setupLabel(
        spaceTypeLabel,
        "SPACE");

    setupLabel(
        spaceSizeLabel,
        "SIZE");

    setupLabel(
        spaceDecayLabel,
        "DECAY");

    setupLabel(
        spacePreDelayLabel,
        "PRE-DELAY");

    setupLabel(
        spaceDampingLabel,
        "DAMPING");

    setupLabel(
        spaceMixLabel,
        "MIX");

    setupComboBox(
        spaceTypeComboBox);

    addAndMakeVisible(
        spaceTypeLabel);

    addAndMakeVisible(
        spaceTypeComboBox);

    populateSpaceComboBox();

    setupSlider(
        spaceSizeSlider);

    setupSlider(
        spaceDecaySlider);

    setupSlider(
        spacePreDelaySlider);

    setupSlider(
        spaceDampingSlider);

    setupSlider(
        spaceMixSlider);

    addAndMakeVisible(
        spaceSizeSlider);

    addAndMakeVisible(
        spaceDecaySlider);

    addAndMakeVisible(
        spacePreDelaySlider);

    addAndMakeVisible(
        spaceDampingSlider);

    addAndMakeVisible(
        spaceMixSlider);

    addAndMakeVisible(
        spaceSizeLabel);

    addAndMakeVisible(
        spaceDecayLabel);

    addAndMakeVisible(
        spacePreDelayLabel);

    addAndMakeVisible(
        spaceDampingLabel);

    addAndMakeVisible(
        spaceMixLabel);

    // ==========================================================
    // ATTACHMENTS
    // ==========================================================

    auto& state =
        audioProcessor.apvts;

    inputAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_INPUT,
                inputSlider);

    globalMixAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_MIX,
                globalMixSlider);

    outputAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_OUTPUT,
                outputSlider);

    bypassAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_BYPASS,
                bypassButton);

    // ==========================================================
    // TUNER ATTACHMENTS
    // ==========================================================

    tunerKeyAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_TUNER_KEY,
                keyComboBox);

    tunerScaleAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_TUNER_SCALE,
                scaleComboBox);

    tunerModeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_TUNER_MODE,
                modeComboBox);

    tunerRetuneAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_TUNER_RETUNE,
                tunerRetuneSlider);

    tunerSmoothAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_TUNER_SMOOTH,
                tunerSmoothSlider);

    tunerFormantAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_TUNER_FORMANT,
                tunerFormantSlider);

    tunerMixAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_TUNER_MIX,
                tunerMixSlider);

    // ==========================================================
    // DOUBLER ATTACHMENTS
    // ==========================================================

    doublerAmountAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_DOUBLER_AMOUNT,
                doublerAmountSlider);

    doublerDetuneAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_DOUBLER_DETUNE,
                doublerDetuneSlider);

    doublerTimingAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_DOUBLER_TIMING,
                doublerTimingSlider);

    doublerWidthAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_DOUBLER_WIDTH,
                doublerWidthSlider);

    doublerMixAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_DOUBLER_MIX,
                doublerMixSlider);

    // ==========================================================
    // HARMONY ATTACHMENTS
    // ==========================================================

    harmonyVoice1Attachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_HARMONY_VOICE1,
                harmonyVoice1ComboBox);

    harmonyVoice2Attachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_HARMONY_VOICE2,
                harmonyVoice2ComboBox);

    harmonyVoice3Attachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_HARMONY_VOICE3,
                harmonyVoice3ComboBox);

    harmonyVoice4Attachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_HARMONY_VOICE4,
                harmonyVoice4ComboBox);

    harmonyMixAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_HARMONY_MIX,
                harmonyMixSlider);

    // ==========================================================
    // CREATIVE FX ATTACHMENTS
    // ==========================================================

    creativeFxTypeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_FX_TYPE,
                creativeFxTypeComboBox);

    creativeFxAmountAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_FX_AMOUNT,
                creativeFxAmountSlider);

    creativeFxMixAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_FX_MIX,
                creativeFxMixSlider);

    // ==========================================================
    // SPACE ATTACHMENTS
    // ==========================================================

    spaceTypeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_SPACE_TYPE,
                spaceTypeComboBox);

    spaceSizeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_SPACE_SIZE,
                spaceSizeSlider);

    spaceDecayAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_SPACE_DECAY,
                spaceDecaySlider);

    spacePreDelayAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_SPACE_PREDELAY,
                spacePreDelaySlider);

    spaceDampingAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_SPACE_DAMPING,
                spaceDampingSlider);

    spaceMixAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                state,
                OfforVocalProAudioProcessor::PARAM_SPACE_MIX,
                spaceMixSlider);

    // ==========================================================
    // PRESET / A-B SYSTEM
    // ==========================================================
    //
    // Set up the controls only after all APVTS attachments
    // have been created.
    //
    // This is important because A/B and presets operate on
    // the complete APVTS state.
    //

    setupPresetControls();


    // ==========================================================
    // LICENSE OVERLAY
    // ==========================================================
    //
    // The overlay starts hidden.
    //
    // timerCallback() will display it when the server reports
    // that all free sessions have been consumed.
    //

    setupLicenseOverlay();

    // ----------------------------------------------------------
    // INITIAL A/B STATES
    // ----------------------------------------------------------
    //
    // At startup both A and B begin with the current plugin
    // settings. From this point the user can change settings
    // and compare them.
    //

    stateA =
        audioProcessor.apvts.copyState();

    stateB =
        audioProcessor.apvts.copyState();

    isAActive = true;

    updateABButtonStates();

    // ==========================================================
    // START
    // ==========================================================

    selectModule(
        Module::tuner);

    startTimerHz(20);
}


// ==========================================================
// SETTINGS PANEL - SHOW
// ==========================================================

void OfforVocalProAudioProcessorEditor::showSettingsPanel()
{
    if (settingsPanel == nullptr)
        return;

    // Make sure it covers the complete plugin window.
    settingsPanel->setBounds(getLocalBounds());

    // Show it above everything else.
    settingsPanel->setVisible(true);
    settingsPanel->toFront(true);

    // Refresh its layout.
    settingsPanel->resized();
    settingsPanel->repaint();
}


// ==========================================================
// SETTINGS PANEL - HIDE
// ==========================================================

void OfforVocalProAudioProcessorEditor::hideSettingsPanel()
{
    if (settingsPanel == nullptr)
        return;

    settingsPanel->setVisible(false);

    // Return keyboard/mouse focus to the main plugin.
    grabKeyboardFocus();
}

//==============================================================================
// PRESET CONTROLS
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupPresetControls()
{
    // ==========================================================
    // PRESET COMBO BOX
    // ==========================================================
    //
    // This stays compact so the header does not become crowded.
    //

    setupComboBox(
        presetComboBox);

    presetComboBox.clear();

    for (int i = 0;
         i < factoryPresetNames.size();
         ++i)
    {
        presetComboBox.addItem(
            factoryPresetNames[i],
            i + 1);
    }

    presetComboBox.setText(
        factoryPresetNames[0],
        juce::dontSendNotification);

    presetComboBox.onChange =
        [this]
        {
            const int selectedPreset =
                presetComboBox.getSelectedId();

            if (selectedPreset > 0)
                loadFactoryPreset(
                    selectedPreset - 1);
        };

    addAndMakeVisible(
        presetComboBox);

    // ==========================================================
    // SAVE BUTTON
    //===========================================================
    // SAVE creates a real user preset file.
    //
    // The complete APVTS state is saved, so all plugin parameters
    // are preserved.
    //
    // A/B remains independent and is used only for comparison.

    savePresetButton.setButtonText(
        "SAVE");

    savePresetButton.setWantsKeyboardFocus(
        false);

    savePresetButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    savePresetButton.onClick =
    [this]
    {
        // SAVE now creates a real preset file on disk.
        //
        // It does NOT overwrite A or B.
        //
        // A/B are for instant comparison.
        // SAVE is for permanent preset storage.
        savePresetToFile();
    };

    addAndMakeVisible(
        savePresetButton);

    // ==========================================================
    // A BUTTON
    // ==========================================================

    aButton.setButtonText(
        "A");

    aButton.setClickingTogglesState(
        false);

    aButton.setWantsKeyboardFocus(
        false);

    aButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    aButton.onClick =
        [this]
        {
            recallStateA();
        };

    addAndMakeVisible(
        aButton);

    // ==========================================================
    // B BUTTON
    // ==========================================================

    bButton.setButtonText(
        "B");

    bButton.setClickingTogglesState(
        false);

    bButton.setWantsKeyboardFocus(
        false);

    bButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    bButton.onClick =
        [this]
        {
            recallStateB();
        };

    addAndMakeVisible(
        bButton);

    updateABButtonStates();
}


//==============================================================================
// LICENSE OVERLAY SETUP
//==============================================================================

void OfforVocalProAudioProcessorEditor::setupLicenseOverlay()
{
    licenseOverlay =
        std::make_unique<LicenseOverlay>(*this);

    addAndMakeVisible(*licenseOverlay);

    // ==========================================================
    // LICENSE OVERLAY
    // ==========================================================
    // Give the overlay the full editor size immediately.
    // This is important because the overlay is supposed to cover
    // the entire plugin window.
    licenseOverlay->setBounds(getLocalBounds());

    // Keep the overlay above all other UI components.
    licenseOverlay->setAlwaysOnTop(true);

    // Start hidden.
    // updateLicenseOverlay() will show it when the trial is used up.
    licenseOverlay->setVisible(false);

    // Force it to the very front of the editor component stack.
    licenseOverlay->toFront(true);
}

//==============================================================================
// LOAD FACTORY PRESET
//==============================================================================

void
OfforVocalProAudioProcessorEditor::loadFactoryPreset(
    int presetIndex)
{
    auto& apvts =
        audioProcessor.apvts;

    // ==========================================================
    // HELPER
    // ==========================================================
    //
    // All APVTS parameters are changed using normalized values.
    //
    // 0.0 = parameter minimum
    // 1.0 = parameter maximum
    //
    // This works for both continuous parameters and choice
    // parameters.
    //

    auto setParameter =
        [&apvts](const juce::String& parameterID,
                 float normalizedValue)
        {
            if (auto* parameter =
                    apvts.getParameter(
                        parameterID))
            {
                parameter->beginChangeGesture();

                parameter->setValueNotifyingHost(
                    juce::jlimit(
                        0.0f,
                        1.0f,
                        normalizedValue));

                parameter->endChangeGesture();
            }
        };

    // ==========================================================
    // START FROM A CLEAN BASELINE
    // ==========================================================
    //
    // These values are intentionally conservative.
    //
    // The goal of the factory presets is not to radically change
    // the sound. They provide useful starting points for vocals.
    //

    switch (presetIndex)
    {
        // ======================================================
        // DEFAULT
        // ======================================================

        case 0:
        {
            setParameter(
                OfforVocalProAudioProcessor::PARAM_INPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_MIX,
                1.00f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_OUTPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_RETUNE,
                0.35f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_SMOOTH,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_FORMANT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_MIX,
                1.00f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_AMOUNT,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_HARMONY_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_TYPE,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_AMOUNT,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_TYPE,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_MIX,
                0.0f);

            break;
        }

        // ======================================================
        // CLEAN VOCAL
        // ======================================================

        case 1:
        {
            setParameter(
                OfforVocalProAudioProcessor::PARAM_INPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_MIX,
                1.00f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_OUTPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_RETUNE,
                0.25f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_SMOOTH,
                0.65f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_FORMANT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_MIX,
                0.80f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_AMOUNT,
                0.10f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_MIX,
                0.10f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_HARMONY_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_TYPE,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_AMOUNT,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_TYPE,
                0.20f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_MIX,
                0.12f);

            break;
        }

        // ======================================================
        // BRIGHT VOCAL
        // ======================================================

        case 2:
        {
            setParameter(
                OfforVocalProAudioProcessor::PARAM_INPUT,
                0.52f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_MIX,
                1.00f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_OUTPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_RETUNE,
                0.35f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_SMOOTH,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_FORMANT,
                0.60f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_MIX,
                0.90f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_AMOUNT,
                0.18f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_MIX,
                0.15f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_HARMONY_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_TYPE,
                0.72f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_AMOUNT,
                0.25f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_MIX,
                0.15f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_TYPE,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_MIX,
                0.18f);

            break;
        }

        // ======================================================
        // WARM VOCAL
        // ======================================================

        case 3:
        {
            setParameter(
                OfforVocalProAudioProcessor::PARAM_INPUT,
                0.48f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_MIX,
                1.00f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_OUTPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_RETUNE,
                0.20f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_SMOOTH,
                0.70f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_FORMANT,
                0.38f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_MIX,
                0.75f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_AMOUNT,
                0.12f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_MIX,
                0.12f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_HARMONY_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_TYPE,
                0.20f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_AMOUNT,
                0.15f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_MIX,
                0.08f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_TYPE,
                0.35f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_MIX,
                0.15f);

            break;
        }

        // ======================================================
        // RADIO VOCAL
        // ======================================================

        case 4:
        {
            setParameter(
                OfforVocalProAudioProcessor::PARAM_INPUT,
                0.55f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_MIX,
                1.00f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_OUTPUT,
                0.48f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_RETUNE,
                0.55f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_SMOOTH,
                0.35f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_FORMANT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_MIX,
                0.85f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_AMOUNT,
                0.05f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_MIX,
                0.05f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_HARMONY_MIX,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_TYPE,
                0.25f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_AMOUNT,
                0.65f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_MIX,
                0.55f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_TYPE,
                0.0f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_MIX,
                0.0f);

            break;
        }

        // ======================================================
        // WIDE VOCAL
        // ======================================================

        case 5:
        {
            setParameter(
                OfforVocalProAudioProcessor::PARAM_INPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_MIX,
                1.00f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_OUTPUT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_RETUNE,
                0.30f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_SMOOTH,
                0.55f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_FORMANT,
                0.50f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_TUNER_MIX,
                0.85f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_AMOUNT,
                0.55f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_DOUBLER_MIX,
                0.35f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_HARMONY_MIX,
                0.25f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_TYPE,
                0.90f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_AMOUNT,
                0.15f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_FX_MIX,
                0.10f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_TYPE,
                0.55f);

            setParameter(
                OfforVocalProAudioProcessor::PARAM_SPACE_MIX,
                0.30f);

            break;
        }

        default:
            return;
    }

    // ==========================================================
    // SAVE PRESET RESULT INTO A
    // ==========================================================
    //
    // Loading a factory preset becomes the new A reference.
    // B keeps its previous state so the user can still compare
    // against the previous B setting.
    //

    stateA =
        audioProcessor.apvts.copyState();

    isAActive = true;

    updateABButtonStates();

    presetComboBox.setText(
        factoryPresetNames[
            juce::jlimit(
                0,
                factoryPresetNames.size() - 1,
                presetIndex)],
        juce::dontSendNotification);
}

//==============================================================================
// SAVE CURRENT STATE TO A
//==============================================================================

void
OfforVocalProAudioProcessorEditor::saveCurrentStateToA()
{
    stateA =
        audioProcessor.apvts.copyState();

    isAActive = true;

    updateABButtonStates();
}

//==============================================================================
// SAVE CURRENT STATE TO B
//==============================================================================

void
OfforVocalProAudioProcessorEditor::saveCurrentStateToB()
{
    stateB =
        audioProcessor.apvts.copyState();

    isAActive = false;

    updateABButtonStates();
}

//==============================================================================
// RECALL A
//==============================================================================

void
OfforVocalProAudioProcessorEditor::recallStateA()
{
    if (!stateA.isValid())
        return;

    audioProcessor.apvts.replaceState(
        stateA.createCopy());

    isAActive = true;

    updateABButtonStates();
}

//==============================================================================
// RECALL B
//==============================================================================

void
OfforVocalProAudioProcessorEditor::recallStateB()
{
    if (!stateB.isValid())
        return;

    audioProcessor.apvts.replaceState(
        stateB.createCopy());

    isAActive = false;

    updateABButtonStates();
}

//==============================================================================
// A/B BUTTON APPEARANCE
//==============================================================================

void
OfforVocalProAudioProcessorEditor::updateABButtonStates()
{
    // ==========================================================
    // A BUTTON
    // ==========================================================

    aButton.setColour(
        juce::TextButton::buttonColourId,
        isAActive
            ? accentColour
            : panelColour2);

    aButton.setColour(
        juce::TextButton::textColourOffId,
        isAActive
            ? textColour
            : mutedColour);

    aButton.setColour(
        juce::TextButton::textColourOnId,
        textColour);

    // ==========================================================
    // B BUTTON
    // ==========================================================

    bButton.setColour(
        juce::TextButton::buttonColourId,
        !isAActive
            ? accentColour
            : panelColour2);

    bButton.setColour(
        juce::TextButton::textColourOffId,
        !isAActive
            ? textColour
            : mutedColour);

    bButton.setColour(
        juce::TextButton::textColourOnId,
        textColour);

    aButton.repaint();
    bButton.repaint();
}

//==============================================================================
// DESTRUCTOR
//==============================================================================

OfforVocalProAudioProcessorEditor::
~OfforVocalProAudioProcessorEditor()
{
    stopTimer();

    // ----------------------------------------------------------
    // Stop license activation before destroying the editor.
    // ----------------------------------------------------------

    if (licenseActivationThread != nullptr)
    {
        licenseActivationThread->stopThread(-1);

        licenseActivationThread.reset();
    }

    presetFileChooser.reset();
}

//==============================================================================
// LOAD IMAGES
//==============================================================================

void
OfforVocalProAudioProcessorEditor::loadImages()
{
    logoImage =
        juce::ImageFileFormat::loadFrom(
            BinaryData::OfforVocalPro_png,
            BinaryData::OfforVocalPro_pngSize);

    radioTunerIcon =
        juce::ImageFileFormat::loadFrom(
            BinaryData::radiotuner_png,
            BinaryData::radiotuner_pngSize);

    doublerIcon =
        juce::ImageFileFormat::loadFrom(
            BinaryData::twoarrows_png,
            BinaryData::twoarrows_pngSize);

    harmonyIcon =
        juce::ImageFileFormat::loadFrom(
            BinaryData::harmonious_png,
            BinaryData::harmonious_pngSize);

    fxIcon =
        juce::ImageFileFormat::loadFrom(
            BinaryData::fxsign_png,
            BinaryData::fxsign_pngSize);

    spaceIcon =
        juce::ImageFileFormat::loadFrom(
            BinaryData::startup_png,
            BinaryData::startup_pngSize);

    settingsIcon =
        juce::ImageFileFormat::loadFrom(
            BinaryData::settings_png,
            BinaryData::settings_pngSize);


    // ==========================================================
    // CONVERT SETTINGS ICON TO WHITE
    // ==========================================================
    //
    // Preserve the original alpha channel but replace the
    // original icon colour with pure white.
    //
    // This allows a black settings PNG to become a clean
    // white hardware-style icon.
    //

    if (settingsIcon.isValid())
    {
        juce::Image whiteSettingsIcon(
            juce::Image::ARGB,
            settingsIcon.getWidth(),
            settingsIcon.getHeight(),
            true);

        whiteSettingsIcon.clear(
            whiteSettingsIcon.getBounds(),
            juce::Colours::transparentBlack);

        juce::Image::BitmapData source(
            settingsIcon,
            juce::Image::BitmapData::readOnly);

        juce::Image::BitmapData destination(
            whiteSettingsIcon,
            juce::Image::BitmapData::readWrite);

        for (int y = 0;
            y < settingsIcon.getHeight();
            ++y)
        {
            for (int x = 0;
                x < settingsIcon.getWidth();
                ++x)
            {
                const juce::Colour sourcePixel =
                    source.getPixelColour(x, y);

                destination.setPixelColour(
                    x,
                    y,
                    juce::Colours::white.withAlpha(
                        sourcePixel.getAlpha()));
            }
        }

        settingsIcon =
            whiteSettingsIcon;
    }
}

//==============================================================================
// SELECT MODULE
//==============================================================================

void
OfforVocalProAudioProcessorEditor::selectModule(
    Module module)
{
    currentModule = module;

    tunerButton.setSelected(
        module == Module::tuner);

    doublerButton.setSelected(
        module == Module::doubler);

    harmonyButton.setSelected(
        module == Module::harmony);

    fxButton.setSelected(
        module == Module::creativeFx);

    spaceButton.setSelected(
        module == Module::space);

    updateModuleVisibility();

    repaint();
}

//==============================================================================
// VISIBILITY
//==============================================================================

void
OfforVocalProAudioProcessorEditor::updateModuleVisibility()
{
    const bool tuner =
        currentModule == Module::tuner;

    const bool doubler =
        currentModule == Module::doubler;

    const bool harmony =
        currentModule == Module::harmony;

    const bool fx =
        currentModule == Module::creativeFx;

    const bool space =
        currentModule == Module::space;

    // ==========================================================
    // TUNER
    // ==========================================================

    // tunerTitleLabel.setVisible(tuner);
    tunerStatusLabel.setVisible(tuner);

    detectedNoteLabel.setVisible(tuner);
    detectedFrequencyLabel.setVisible(tuner);
    detectedCentsLabel.setVisible(tuner);
    detectedConfidenceLabel.setVisible(tuner);

    keyLabel.setVisible(tuner);
    scaleLabel.setVisible(tuner);
    modeLabel.setVisible(tuner);

    keyComboBox.setVisible(tuner);
    scaleComboBox.setVisible(tuner);
    modeComboBox.setVisible(tuner);

    tunerRetuneLabel.setVisible(tuner);
    tunerSmoothLabel.setVisible(tuner);
    tunerFormantLabel.setVisible(tuner);
    tunerMixLabel.setVisible(tuner);

    tunerRetuneSlider.setVisible(tuner);
    tunerSmoothSlider.setVisible(tuner);
    tunerFormantSlider.setVisible(tuner);
    tunerMixSlider.setVisible(tuner);

    // ==========================================================
    // DOUBLER
    // ==========================================================

    // doublerTitleLabel.setVisible(doubler);

    doublerAmountSlider.setVisible(doubler);
    doublerDetuneSlider.setVisible(doubler);
    doublerTimingSlider.setVisible(doubler);
    doublerWidthSlider.setVisible(doubler);
    doublerMixSlider.setVisible(doubler);

    doublerAmountLabel.setVisible(doubler);
    doublerDetuneLabel.setVisible(doubler);
    doublerTimingLabel.setVisible(doubler);
    doublerWidthLabel.setVisible(doubler);
    doublerMixLabel.setVisible(doubler);

    // ==========================================================
    // HARMONY
    // ==========================================================

    // harmonyTitleLabel.setVisible(harmony);

    harmonyVoice1Label.setVisible(harmony);
    harmonyVoice2Label.setVisible(harmony);
    harmonyVoice3Label.setVisible(harmony);
    harmonyVoice4Label.setVisible(harmony);

    harmonyVoice1ComboBox.setVisible(harmony);
    harmonyVoice2ComboBox.setVisible(harmony);
    harmonyVoice3ComboBox.setVisible(harmony);
    harmonyVoice4ComboBox.setVisible(harmony);

    harmonyMixLabel.setVisible(harmony);
    harmonyMixSlider.setVisible(harmony);

    // ==========================================================
    // CREATIVE FX
    // ==========================================================

    // creativeFxTitleLabel.setVisible(fx);

    creativeFxTypeLabel.setVisible(fx);
    creativeFxAmountLabel.setVisible(fx);
    creativeFxMixLabel.setVisible(fx);

    creativeFxTypeComboBox.setVisible(fx);
    creativeFxAmountSlider.setVisible(fx);
    creativeFxMixSlider.setVisible(fx);

    // ==========================================================
    // SPACE
    // ==========================================================

    // spaceTitleLabel.setVisible(space);

    spaceTypeLabel.setVisible(space);
    spaceSizeLabel.setVisible(space);
    spaceDecayLabel.setVisible(space);
    spacePreDelayLabel.setVisible(space);
    spaceDampingLabel.setVisible(space);
    spaceMixLabel.setVisible(space);

    spaceTypeComboBox.setVisible(space);

    spaceSizeSlider.setVisible(space);
    spaceDecaySlider.setVisible(space);
    spacePreDelaySlider.setVisible(space);
    spaceDampingSlider.setVisible(space);
    spaceMixSlider.setVisible(space);
}

//==============================================================================
// PAINT
//==============================================================================

void
OfforVocalProAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    g.fillAll(
        backgroundColour);

    const auto area =
        getLocalBounds();

    // ==========================================================
    // HEADER
    // ==========================================================

    auto header =
        area.withHeight(64);

    g.setColour(
        juce::Colour::fromString(
            "FF0D0F11"));

    g.fillRect(header);

    g.setColour(
        borderColour);

    g.drawLine(
        0.0f,
        63.0f,
        static_cast<float>(getWidth()),
        63.0f,
        1.0f);

    // ==========================================================
    // LOGO
    // ==========================================================

    if (logoImage.isValid())
    {
        g.drawImageWithin(
            logoImage,
            18,
            14,
            38,
            38,
            juce::RectanglePlacement::centred,
            false);
    }

    // ==========================================================
    // MAIN PANEL
    // ==========================================================

    const int contentTop = 76;

    auto mainPanel =
        juce::Rectangle<int>(
            196,
            contentTop,
            getWidth() - 212,
            getHeight() - contentTop - 98);

    drawPanel(
        g,
        mainPanel,
        true);

    // ==========================================================
    // MODULE HEADER
    // ==========================================================

    juce::String moduleName;

    switch (currentModule)
    {
        case Module::tuner:
            moduleName = "PITCH TUNER";
            break;

        case Module::doubler:
            moduleName = "VOCAL DOUBLER";
            break;

        case Module::harmony:
            moduleName = "VOCAL HARMONY";
            break;

        case Module::creativeFx:
            moduleName = "CREATIVE FX";
            break;

        case Module::space:
            moduleName = "VOCAL SPACE";
            break;
    }

    drawModuleHeader(
        g,
        mainPanel,
        moduleName);

    // ==========================================================
    // TUNER PITCH METER
    // ==========================================================

    if (currentModule == Module::tuner)
    {
        auto meterBounds =
            juce::Rectangle<int>(
                mainPanel.getX() + 22,
                mainPanel.getY() + 62,
                mainPanel.getWidth() - 44,
                110);

        drawPitchMeter(
            g,
            meterBounds);
    }

    // ==========================================================
    // GLOBAL FOOTER
    // ==========================================================

    auto global =
    juce::Rectangle<int>(
        196,
        getHeight() - 88,
        getWidth() - 212,
        72);

    drawGlobalSection(
        g,
        global);

    // ==========================================================
    // LEFT MODULE RAIL
    // ==========================================================

    auto rail =
        juce::Rectangle<int>(
            14,
            contentTop,
            166,
            getHeight() - contentTop - 74);

    g.setColour(
        panelColour);

    g.fillRoundedRectangle(
        rail.toFloat(),
        10.0f);

    g.setColour(
        borderColour);

    g.drawRoundedRectangle(
        rail.toFloat(),
        10.0f,
        1.0f);

    g.setColour(
        mutedColour);

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(8.0f)
                .withStyle("Bold")));

    g.drawText(
        "VOCAL CHAIN",
        rail.getX() + 15,
        rail.getY() + 12,
        rail.getWidth() - 30,
        18,
        juce::Justification::centredLeft);
}

//==============================================================================
// RESIZED
//==============================================================================

void
OfforVocalProAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();

    // ==========================================================
    // HEADER
    // ==========================================================

    titleLabel.setBounds(
        68,
        11,
        240,
        24);

    subtitleLabel.setBounds(
        69,
        35,
        180,
        14);

    // ----------------------------------------------------------
    // HEADER CONTROL LAYOUT
    // ----------------------------------------------------------
    //
    // Keep the controls compact and aligned on the right.
    //
    // PRESET -> SAVE -> A -> B -> SETTINGS -> BYPASS
    //
    // The version label is moved closer to the title so it does
    // not compete with the preset controls.
    //

    versionLabel.setBounds(
        310,
        20,
        55,
        20);

    bypassButton.setBounds(
        w - 112,
        17,
        90,
        30);

    settingsButton.setBounds(
        w - 205,
        16,
        48,
        32);

    // ==========================================================
    // PRESET / A-B HEADER CONTROLS
    // ==========================================================
    //
    // Compact controls placed between the title area and the
    // SETTINGS/BYPASS controls.
    //
    // SAVE writes the current APVTS state to a preset file.
    //
    // A/B are instant comparison snapshots.
    //

    const int presetY = 17;

    presetComboBox.setBounds(
        w - 525,
        presetY,
        155,
        30);

    savePresetButton.setBounds(
        w - 360,
        presetY,
        55,
        30);

    aButton.setBounds(
        w - 295,
        presetY,
        30,
        30);

    bButton.setBounds(
        w - 258,
        presetY,
        30,
        30);

    // ==========================================================
    // MODULE RAIL
    // ==========================================================

    const int railX = 22;
    const int railY = 110;
    const int railWidth = 150;
    const int buttonHeight = 56;
    const int gap = 8;

    tunerButton.setBounds(
        railX,
        railY,
        railWidth,
        buttonHeight);

    doublerButton.setBounds(
        railX,
        railY + (buttonHeight + gap),
        railWidth,
        buttonHeight);

    harmonyButton.setBounds(
        railX,
        railY + (buttonHeight + gap) * 2,
        railWidth,
        buttonHeight);

    fxButton.setBounds(
        railX,
        railY + (buttonHeight + gap) * 3,
        railWidth,
        buttonHeight);

    spaceButton.setBounds(
        railX,
        railY + (buttonHeight + gap) * 4,
        railWidth,
        buttonHeight);

    // ==========================================================
    // MAIN PANEL
    // ==========================================================

    const int panelX = 196;
    const int panelY = 76;
    const int panelW = w - 212;
    const int panelH = h - panelY - 98;

    // ==========================================================
    // MAIN PANEL CONTENT AREA
    // ==========================================================
    //
    // Reserve the right side of the panel for the INPUT/OUTPUT
    // level meters. This prevents module controls from extending
    // underneath the meters.
    //

    const int innerX = panelX + 24;

    // Width reserved for the two vertical meters plus:
    // - 10 px gap between meters
    // - 22 px right margin
    // - 24 px breathing room before the meters
    //
    const int meterReservedWidth = 48 + 10 + 48 + 22 + 24;

    const int innerW =
        panelW - 48 - meterReservedWidth;

    // ==========================================================
    // TUNER
    // ==========================================================

    // tunerTitleLabel.setBounds(
    //     innerX,
    //     panelY + 14,
    //     200,
    //     24);

    tunerStatusLabel.setBounds(
        panelX + panelW - 120,
        panelY + 16,
        90,
        20);

    detectedNoteLabel.setBounds(
        innerX,
        panelY + 70,
        180,
        68);

    detectedFrequencyLabel.setBounds(
        innerX,
        panelY + 139,
        160,
        22);

    detectedCentsLabel.setBounds(
        innerX,
        panelY + 163,
        160,
        22);

    detectedConfidenceLabel.setBounds(
        innerX,
        panelY + 187,
        180,
        22);

    const int selectY =
        panelY + 230;

    const int selectWidth =
        juce::jmax(
            105,
            (innerW - 20) / 3);

    keyLabel.setBounds(
        innerX,
        selectY,
        selectWidth,
        15);

    keyComboBox.setBounds(
        innerX,
        selectY + 17,
        selectWidth,
        32);

    scaleLabel.setBounds(
        innerX + selectWidth + 10,
        selectY,
        selectWidth,
        15);

    scaleComboBox.setBounds(
        innerX + selectWidth + 10,
        selectY + 17,
        selectWidth,
        32);

    modeLabel.setBounds(
        innerX + (selectWidth + 10) * 2,
        selectY,
        selectWidth,
        15);

    modeComboBox.setBounds(
        innerX + (selectWidth + 10) * 2,
        selectY + 17,
        selectWidth,
        32);

    const int tunerKnobY =
        panelY + panelH - 115;

    const int tunerKnobWidth =
        juce::jmax(
            70,
            (innerW - 36) / 4);

    ProfessionalKnob* tunerSliders[] =
    {
        &tunerRetuneSlider,
        &tunerSmoothSlider,
        &tunerFormantSlider,
        &tunerMixSlider
    };

    juce::Label* tunerLabels[] =
    {
        &tunerRetuneLabel,
        &tunerSmoothLabel,
        &tunerFormantLabel,
        &tunerMixLabel
    };

    for (int i = 0; i < 4; ++i)
    {
        const int x =
            innerX + i * (tunerKnobWidth + 12);

        tunerLabels[i]->setBounds(
            x,
            tunerKnobY,
            tunerKnobWidth,
            16);

        tunerSliders[i]->setBounds(
            x,
            tunerKnobY + 12,
            tunerKnobWidth,
            80);
    }

    // ==========================================================
    // DOUBLER
    // ==========================================================

    // Three controls on the first row.
    // Two controls on the second row.
    //
    // This prevents the knobs from entering the meter area
    // and gives the Doubler module a cleaner hardware layout.

    const int dY =
        panelY + 95;

    const int dGap = 18;

    const int dTopWidth =
        juce::jmax(
            80,
            (innerW - dGap * 2) / 3);

    const int dBottomWidth =
        juce::jmax(
            80,
            (innerW - dGap) / 2);

    ProfessionalKnob* dSliders[] =
    {
        &doublerAmountSlider,
        &doublerDetuneSlider,
        &doublerTimingSlider,
        &doublerWidthSlider,
        &doublerMixSlider
    };

    juce::Label* dLabels[] =
    {
        &doublerAmountLabel,
        &doublerDetuneLabel,
        &doublerTimingLabel,
        &doublerWidthLabel,
        &doublerMixLabel
    };

    // ----------------------------------------------------------
    // TOP ROW
    // ----------------------------------------------------------

    for (int i = 0; i < 3; ++i)
    {
        const int x =
            innerX + i * (dTopWidth + dGap);

        dLabels[i]->setBounds(
            x,
            dY,
            dTopWidth,
            18);

        dSliders[i]->setBounds(
            x,
            dY + 18,
            dTopWidth,
            90);
    }

    // ----------------------------------------------------------
    // BOTTOM ROW
    // ----------------------------------------------------------

    const int bottomY =
        dY + 125;

    for (int i = 0; i < 2; ++i)
    {
        const int x =
            innerX + i * (dBottomWidth + dGap);

        dLabels[i + 3]->setBounds(
            x,
            bottomY,
            dBottomWidth,
            18);

        dSliders[i + 3]->setBounds(
            x,
            bottomY + 18,
            dBottomWidth,
            90);
    }

    // ==========================================================
    // HARMONY
    // ==========================================================
    //
    // Keep the voice selectors compact so they remain well away
    // from the right-side level meters.
    //

    const int harmonyY =
        panelY + 88;

    const int harmonyRowHeight =
        43;

    const int harmonyLabelWidth =
        72;

    const int harmonyBoxWidth =
        juce::jmin(
            250,
            innerW - harmonyLabelWidth - 10);

    juce::ComboBox* harmonyBoxes[] =
    {
        &harmonyVoice1ComboBox,
        &harmonyVoice2ComboBox,
        &harmonyVoice3ComboBox,
        &harmonyVoice4ComboBox
    };

    juce::Label* harmonyLabels[] =
    {
        &harmonyVoice1Label,
        &harmonyVoice2Label,
        &harmonyVoice3Label,
        &harmonyVoice4Label
    };

    for (int i = 0; i < 4; ++i)
    {
        const int y =
            harmonyY + i * harmonyRowHeight;

        harmonyLabels[i]->setBounds(
            innerX,
            y,
            harmonyLabelWidth,
            32);

        harmonyBoxes[i]->setBounds(
            innerX + harmonyLabelWidth + 10,
            y,
            harmonyBoxWidth,
            34);
    }

    // ----------------------------------------------------------
    // HARMONY MIX
    // ----------------------------------------------------------

    harmonyMixLabel.setBounds(
        innerX,
        harmonyY + 180,
        120,
        18);

    harmonyMixSlider.setBounds(
        innerX,
        harmonyY + 200,
        160,
        90);

    // ==========================================================
    // CREATIVE FX
    // ==========================================================

    // Keep the effect selector inside the module content area.
    // The right side is reserved for the level meters.

    creativeFxTypeLabel.setBounds(
        innerX,
        panelY + 96,
        100,
        18);

    const int fxComboWidth =
        juce::jmin(
            300,
            innerW);

    creativeFxTypeComboBox.setBounds(
        innerX,
        panelY + 120,
        fxComboWidth,
        38);

    const int fxKnobY =
        panelY + 195;

    creativeFxAmountLabel.setBounds(
        innerX,
        fxKnobY,
        100,
        18);

    creativeFxAmountSlider.setBounds(
        innerX,
        fxKnobY + 18,
        150,
        105);

    creativeFxMixLabel.setBounds(
        innerX + 190,
        fxKnobY,
        100,
        18);

    creativeFxMixSlider.setBounds(
        innerX + 190,
        fxKnobY + 18,
        150,
        105);

    // ==========================================================
    // SPACE
    // ==========================================================

    // Keep the Space type selector compact and away from the
    // right-side level meters.

    spaceTypeLabel.setBounds(
        innerX,
        panelY + 96,
        80,
        18);

    const int spaceComboWidth =
        juce::jmin(
            300,
            innerW);

    spaceTypeComboBox.setBounds(
        innerX,
        panelY + 120,
        spaceComboWidth,
        38);

    const int sY =
        panelY + 205;

    const int sGap = 14;

    const int sWidth =
        juce::jmax(
            70,
            (innerW - sGap * 4) / 5);

    ProfessionalKnob* sSliders[] =
    {
        &spaceSizeSlider,
        &spaceDecaySlider,
        &spacePreDelaySlider,
        &spaceDampingSlider,
        &spaceMixSlider
    };

    juce::Label* sLabels[] =
    {
        &spaceSizeLabel,
        &spaceDecayLabel,
        &spacePreDelayLabel,
        &spaceDampingLabel,
        &spaceMixLabel
    };

    for (int i = 0; i < 5; ++i)
    {
        const int x =
            innerX + i * (sWidth + sGap);

        sLabels[i]->setBounds(
            x,
            sY,
            sWidth,
            18);

        sSliders[i]->setBounds(
            x,
            sY + 18,
            sWidth,
            100);
    }

    // ==========================================================
    // GLOBAL HARDWARE KNOBS
    // ==========================================================

    const int footerY =
        h - 88;

    const int footerX =
        196;

    const int footerW =
        w - 212;

    const int globalKnobSize =
        64;

    const int globalGroupWidth =
        footerW / 3;

    // ----------------------------------------------------------
    // INPUT
    // ----------------------------------------------------------

    inputLabel.setBounds(
        footerX,
        footerY + 2,
        globalGroupWidth,
        16);

    inputSlider.setBounds(
        footerX
            + (globalGroupWidth - globalKnobSize) / 2,
        footerY + 15,
        globalKnobSize,
        54);

    // ----------------------------------------------------------
    // MIX
    // ----------------------------------------------------------

    globalMixLabel.setBounds(
        footerX + globalGroupWidth,
        footerY + 2,
        globalGroupWidth,
        16);

    globalMixSlider.setBounds(
        footerX
            + globalGroupWidth
            + (globalGroupWidth - globalKnobSize) / 2,
        footerY + 15,
        globalKnobSize,
        54);

    // ----------------------------------------------------------
    // OUTPUT
    // ----------------------------------------------------------

    outputLabel.setBounds(
        footerX + globalGroupWidth * 2,
        footerY + 2,
        globalGroupWidth,
        16);

    outputSlider.setBounds(
        footerX
            + globalGroupWidth * 2
            + (globalGroupWidth - globalKnobSize) / 2,
        footerY + 15,
        globalKnobSize,
        54);

    // ==========================================================
    // INPUT / OUTPUT LEVEL METERS
    // ==========================================================
    //
    // Two vertical meters standing shoulder-to-shoulder.
    // They live on the right side of the main panel.
    //
    // IMPORTANT:
    // These do not interfere with the existing INPUT/MIX/OUTPUT
    // knobs in the footer.
    //

    const int meterWidth = 48;
    const int meterHeight = 150;
    const int meterGap = 10;

    const int meterRightMargin = 22;

    const int metersTotalWidth =
        meterWidth * 2 + meterGap;

    const int metersX =
        panelX
        + panelW
        - meterRightMargin
        - metersTotalWidth;

    const int metersY =
        panelY + 82;

    inputMeter.setBounds(
        metersX,
        metersY,
        meterWidth,
        meterHeight);

    outputMeter.setBounds(
        metersX + meterWidth + meterGap,
        metersY,
        meterWidth,
        meterHeight);

     // ==========================================================
    // LICENSE OVERLAY
    // ==========================================================
    // The overlay must always cover the complete editor.
    // Keep this at the END of resized().
    if (licenseOverlay != nullptr)
    {
        licenseOverlay->setBounds(getLocalBounds());

        // Make absolutely sure it stays above the plugin UI.
        licenseOverlay->toFront(true);
    }

    // ==========================================================
    // SETTINGS PANEL
    // ==========================================================
    // Keep the settings screen exactly the same size as the
    // plugin editor and above all other components.
    if (settingsPanel != nullptr)
    {
        settingsPanel->setBounds(getLocalBounds());

        if (settingsPanel->isVisible())
            settingsPanel->toFront(true);
    }

}

//==============================================================================
// PROFESSIONAL HARDWARE KNOB
//==============================================================================

OfforVocalProAudioProcessorEditor::ProfessionalKnob::
ProfessionalKnob()
{
    setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        68,
        18);

    setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    setWantsKeyboardFocus(false);

    setColour(
        juce::Slider::textBoxTextColourId,
        juce::Colour(0xffff7a18));

    setColour(
        juce::Slider::textBoxBackgroundColourId,
        juce::Colour(0xff08090a));

    setColour(
        juce::Slider::textBoxOutlineColourId,
        juce::Colours::transparentBlack);

    setNumDecimalPlacesToDisplay(1);
}


//===================================
// PROFESSIONAL KNOB PAINT
//===========================================

void
OfforVocalProAudioProcessorEditor::ProfessionalKnob::
paint(
    juce::Graphics& g)
{
    auto bounds =
        getLocalBounds().toFloat();

    // ------------------------------------------------------
    // KNOB AREA
    // ------------------------------------------------------

    auto knobArea =
        bounds;

    // Leave room for the value box.
    knobArea.removeFromBottom(20.0f);

    auto knobBounds =
        knobArea.reduced(5.0f);

    const float size =
        juce::jmin(
            knobBounds.getWidth(),
            knobBounds.getHeight());

    knobBounds =
        juce::Rectangle<float>(
            knobBounds.getCentreX() - size * 0.5f,
            knobBounds.getCentreY() - size * 0.5f,
            size,
            size);

    const float centreX =
        knobBounds.getCentreX();

    const float centreY =
        knobBounds.getCentreY();

    const float radius =
        knobBounds.getWidth() * 0.5f;

    // ------------------------------------------------------
    // VALUE
    // ------------------------------------------------------

    const double range =
        getMaximum() - getMinimum();

    const double normalized =
        range > 0.0
            ? juce::jlimit(
                  0.0,
                  1.0,
                  (getValue() - getMinimum()) / range)
            : 0.0;

    // Approximately 270 degree sweep.
    const float startAngle =
        juce::MathConstants<float>::pi * 1.25f;

    const float endAngle =
        juce::MathConstants<float>::pi * 2.75f;

    const float valueAngle =
        startAngle
        + static_cast<float>(normalized)
            * (endAngle - startAngle);

    // ======================================================
    // OFFOR DARK / RED-ORANGE PALETTE
    // ======================================================

    const auto redOrange =
        juce::Colour(0xffff3b1f);

    const auto brightRedOrange =
        juce::Colour(0xffff5a36);

    const auto darkOuter =
        juce::Colour(0xff050607);

    const auto darkBody =
        juce::Colour(0xff111214);

    const auto innerFace =
        juce::Colour(0xff191b1e);

    const auto darkRedOrange =
        juce::Colour(0xff4a1710);

    // ======================================================
    // OUTER SHADOW
    // ======================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.75f));

    g.fillEllipse(
        knobBounds.translated(
            0.0f,
            2.5f));

    // ======================================================
    // OUTER HARDWARE BODY
    // ======================================================

    juce::ColourGradient outerGradient(
        juce::Colour(0xff252321),
        knobBounds.getCentreX(),
        knobBounds.getY(),

        darkOuter,
        knobBounds.getCentreX(),
        knobBounds.getBottom(),

        false);

    g.setGradientFill(
        outerGradient);

    g.fillEllipse(
        knobBounds);

    // ======================================================
    // OUTER EDGE
    // ======================================================

    g.setColour(
        juce::Colour(0xff3a2522));

    g.drawEllipse(
        knobBounds,
        1.2f);

    // Lower dark machining edge
    g.setColour(
        juce::Colours::black.withAlpha(0.8f));

    g.drawEllipse(
        knobBounds.reduced(2.0f),
        1.0f);

    // ======================================================
    // INNER RECESSED FACE
    // ======================================================

    auto innerBounds =
        knobBounds.reduced(
            radius * 0.105f);

    juce::ColourGradient faceGradient(
        juce::Colour(0xff272522),
        innerBounds.getX(),
        innerBounds.getY(),

        innerFace,
        innerBounds.getRight(),
        innerBounds.getBottom(),

        true);

    g.setGradientFill(
        faceGradient);

    g.fillEllipse(
        innerBounds);

    // ======================================================
    // INNER EDGE
    // ======================================================

    g.setColour(
        juce::Colour(0xff3a2522)
            .withAlpha(0.45f));

    g.drawEllipse(
        innerBounds.reduced(1.0f),
        1.0f);

    // ======================================================
    // VALUE ARC BACKGROUND
    // ======================================================

    const float arcRadius =
        radius * 0.84f;

    juce::Path backgroundArc;

    backgroundArc.addCentredArc(
        centreX,
        centreY,
        arcRadius,
        arcRadius,
        0.0f,
        startAngle,
        endAngle,
        true);

    g.setColour(
        darkRedOrange.withAlpha(0.9f));

    g.strokePath(
        backgroundArc,
        juce::PathStrokeType(
            2.5f,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

    // ======================================================
    // ACTIVE RED-ORANGE ARC
    // ======================================================

    if (normalized > 0.0001)
    {
        juce::Path activeArc;

        activeArc.addCentredArc(
            centreX,
            centreY,
            arcRadius,
            arcRadius,
            0.0f,
            startAngle,
            valueAngle,
            true);

        g.setColour(
            redOrange);

        g.strokePath(
            activeArc,
            juce::PathStrokeType(
                3.2f,
                juce::PathStrokeType::curved,
                juce::PathStrokeType::rounded));
    }

    // ======================================================
    // POSITION INDICATOR
    // ======================================================

    const float indicatorRadius =
        radius * 0.72f;

    const float indicatorX =
        centreX
        + std::cos(valueAngle)
            * indicatorRadius;

    const float indicatorY =
        centreY
        + std::sin(valueAngle)
            * indicatorRadius;

    // Soft red-orange glow
    g.setColour(
        redOrange.withAlpha(0.13f));

    g.fillEllipse(
        indicatorX - 6.0f,
        indicatorY - 6.0f,
        12.0f,
        12.0f);

    // Main indicator dot
    g.setColour(
        brightRedOrange);

    g.fillEllipse(
        indicatorX - 3.0f,
        indicatorY - 3.0f,
        6.0f,
        6.0f);

    // ======================================================
    // CENTER CAP
    // ======================================================

    const float capRadius =
        radius * 0.075f;

    g.setColour(
        juce::Colours::black.withAlpha(0.8f));

    g.fillEllipse(
        centreX - capRadius,
        centreY - capRadius,
        capRadius * 2.0f,
        capRadius * 2.0f);

    g.setColour(
        brightRedOrange.withAlpha(0.20f));

    g.drawEllipse(
        centreX - capRadius,
        centreY - capRadius,
        capRadius * 2.0f,
        capRadius * 2.0f,
        0.7f);
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::ProfessionalKnob::
mouseEnter(
    const juce::MouseEvent& event)
{
    isActive = true;
    repaint();

    juce::Slider::mouseEnter(event);
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::ProfessionalKnob::
mouseExit(
    const juce::MouseEvent& event)
{
    if (!isMouseButtonDown())
        isActive = false;

    repaint();

    juce::Slider::mouseExit(event);
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::ProfessionalKnob::
mouseDown(
    const juce::MouseEvent& event)
{
    isActive = true;
    repaint();

    juce::Slider::mouseDown(event);
}


//==============================================================================

void
OfforVocalProAudioProcessorEditor::ProfessionalKnob::
mouseUp(
    const juce::MouseEvent& event)
{
    juce::Slider::mouseUp(event);

    isActive = isMouseOver();
    repaint();
}

//==============================================================================
// PROFESSIONAL KNOB SETUP
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupSlider(
    juce::Slider& slider)
{
    slider.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    slider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        68,
        18);

    slider.setColour(
        juce::Slider::textBoxTextColourId,
        textColour);

    slider.setColour(
        juce::Slider::textBoxBackgroundColourId,
        displayColour);

    slider.setColour(
        juce::Slider::textBoxOutlineColourId,
        borderColour);

    slider.setColour(
        juce::Slider::textBoxHighlightColourId,
        accentColour);

    slider.setNumDecimalPlacesToDisplay(
        1);
}

//==============================================================================
// GLOBAL HARDWARE KNOB
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupGlobalSlider(
    juce::Slider& slider)
{
    slider.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    slider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        68,
        18);

    slider.setColour(
        juce::Slider::textBoxTextColourId,
        textColour);

    slider.setColour(
        juce::Slider::textBoxBackgroundColourId,
        displayColour);

    slider.setColour(
        juce::Slider::textBoxOutlineColourId,
        borderColour);

    slider.setColour(
        juce::Slider::textBoxHighlightColourId,
        accentColour);

    slider.setNumDecimalPlacesToDisplay(
        1);
}

//==============================================================================
// LABEL
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupLabel(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(
        text,
        juce::dontSendNotification);

    label.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(9.0f)
                .withStyle("Bold")));

    label.setColour(
        juce::Label::textColourId,
        mutedColour);

    label.setJustificationType(
        juce::Justification::centredLeft);
}

//==============================================================================
// TITLE
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupTitle(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(
        text,
        juce::dontSendNotification);

    label.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(13.0f)
                .withStyle("Bold")));

    label.setColour(
        juce::Label::textColourId,
        textColour);

    label.setJustificationType(
        juce::Justification::centredLeft);
}

//==============================================================================
// COMBO
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupComboBox(
    juce::ComboBox& comboBox)
{
    comboBox.setColour(
        juce::ComboBox::backgroundColourId,
        displayColour);

    comboBox.setColour(
        juce::ComboBox::outlineColourId,
        borderColour);

    comboBox.setColour(
        juce::ComboBox::textColourId,
        textColour);

    comboBox.setColour(
        juce::ComboBox::arrowColourId,
        mutedColour);

    comboBox.setJustificationType(
        juce::Justification::centredLeft);

    comboBox.setWantsKeyboardFocus(
        false);
}

//==============================================================================
// PITCH LABEL
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupPitchDisplayLabel(
    juce::Label& label)
{
    label.setText(
        "--",
        juce::dontSendNotification);

    label.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(11.0f)
                .withStyle("Bold")));

    label.setColour(
        juce::Label::textColourId,
        mutedColour);

    label.setJustificationType(
        juce::Justification::centredLeft);
}

//==============================================================================
// MODULE BUTTON
//==============================================================================

void
OfforVocalProAudioProcessorEditor::setupModuleButton(
    ModuleButton& button,
    const juce::Image& icon,
    const juce::String& name)
{
    button.setIcon(
        icon);

    button.setModuleName(
        name);

    addAndMakeVisible(
        button);
}

//==============================================================================
// PANEL
//==============================================================================

void
OfforVocalProAudioProcessorEditor::drawPanel(
    juce::Graphics& g,
    juce::Rectangle<int> bounds,
    bool highlighted)
{
    g.setColour(
        highlighted
            ? panelColour2
            : panelColour);

    g.fillRoundedRectangle(
        bounds.toFloat(),
        12.0f);

    g.setColour(
        highlighted
            ? juce::Colour::fromString("FF30353B")
            : borderColour);

    g.drawRoundedRectangle(
        bounds.toFloat(),
        12.0f,
        1.0f);
}

//==============================================================================
// MODULE HEADER
//==============================================================================

void
OfforVocalProAudioProcessorEditor::drawModuleHeader(
    juce::Graphics& g,
    juce::Rectangle<int> bounds,
    const juce::String& title)
{
    g.setColour(
        textColour);

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(11.0f)
                .withStyle("Bold")));

    g.drawText(
        title,
        bounds.getX() + 24,
        bounds.getY() + 15,
        220,
        22,
        juce::Justification::centredLeft);

    g.setColour(
        accentColour);

    g.fillRoundedRectangle(
        juce::Rectangle<float>(
            static_cast<float>(bounds.getX() + 24),
            static_cast<float>(bounds.getY() + 41),
            38.0f,
            2.0f),
        1.0f);

    g.setColour(
        borderColour);

    g.drawLine(
        static_cast<float>(bounds.getX() + 24),
        static_cast<float>(bounds.getY() + 50),
        static_cast<float>(bounds.getRight() - 24),
        static_cast<float>(bounds.getY() + 50),
        1.0f);
}

//==============================================================================
// FUNCTIONAL PITCH VISUALIZER
//==============================================================================
//
// This is the visual pitch accuracy display used by the TUNER.
//
// The horizontal position of the orange indicator represents
// the detected pitch error:
//
//     LEFT  = FLAT
//     CENTER = IN TUNE
//     RIGHT = SHARP
//
// The actual pitch information comes from:
//
//     audioProcessor.getDetectedCents()
//
// The GUI does not calculate pitch itself.
// It only visualizes the pitch data already produced by
// the processor.
//
//==============================================================================

void
OfforVocalProAudioProcessorEditor::drawPitchMeter(
    juce::Graphics& g,
    juce::Rectangle<int> bounds)
{
    // ==========================================================
    // GET CURRENT PITCH INFORMATION
    // ==========================================================

    const bool hasPitch =
        audioProcessor.hasDetectedPitch();

    const double cents =
        audioProcessor.getDetectedCents();

    // ==========================================================
    // BASIC GEOMETRY
    // ==========================================================

    const float centreX =
        static_cast<float>(
            bounds.getCentreX());

    const float centreY =
        static_cast<float>(
            bounds.getCentreY());

    const float radius =
        juce::jmin(
            bounds.getWidth() * 0.28f,
            bounds.getHeight() * 0.48f);

    // ==========================================================
    // COLORS
    // ==========================================================

    const auto background =
        displayColour;

    const auto border =
        borderColour;

    const auto accent =
        accentColour;

    const auto inactive =
        mutedColour;

    // ==========================================================
    // MAIN CIRCLE
    // ==========================================================

    g.setColour(background);

    g.fillEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f);

    g.setColour(border);

    g.drawEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    // ==========================================================
    // PITCH RANGE
    // ==========================================================
    //
    // We visualize approximately -50 to +50 cents.
    //
    // Anything beyond this range is clamped to the edge.
    //

    const double clampedCents =
        juce::jlimit(
            -50.0,
            50.0,
            cents);

    const float normalized =
        static_cast<float>(
            (clampedCents + 50.0) / 100.0);

    // ==========================================================
    // CENTER AXIS
    // ==========================================================
    //
    // This represents perfect pitch.
    //

    g.setColour(
        border.withAlpha(0.9f));

    g.drawLine(
        centreX - radius * 0.72f,
        centreY,
        centreX + radius * 0.72f,
        centreY,
        1.5f);

    // ==========================================================
    // PITCH TICKS
    // ==========================================================

    for (int i = -4; i <= 4; ++i)
    {
        const float tickX =
            centreX
            + (static_cast<float>(i) / 4.0f)
              * radius * 0.72f;

        const float tickHeight =
            (i == 0)
                ? 10.0f
                : 6.0f;

        g.setColour(
            i == 0
                ? accent
                : border);

        g.drawLine(
            tickX,
            centreY - tickHeight,
            tickX,
            centreY + tickHeight,
            i == 0 ? 2.0f : 1.0f);
    }

    // ==========================================================
    // FLAT / SHARP LABELS
    // ==========================================================

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(8.0f)
                .withStyle("Bold")));

    g.setColour(
        inactive.withAlpha(0.8f));

    g.drawText(
        "FLAT",
        static_cast<int>(
            centreX - radius * 0.82f),
        static_cast<int>(
            centreY + radius * 0.48f),
        45,
        14,
        juce::Justification::centred);

    g.drawText(
        "SHARP",
        static_cast<int>(
            centreX + radius * 0.40f),
        static_cast<int>(
            centreY + radius * 0.48f),
        45,
        14,
        juce::Justification::centred);

    // ==========================================================
    // NO PITCH
    // ==========================================================
    //
    // When there is no detected vocal, show a neutral center
    // indicator instead of pretending that the signal is in tune.
    //

    if (!hasPitch)
    {
        g.setColour(
            inactive.withAlpha(0.55f));

        g.fillEllipse(
            centreX - 4.0f,
            centreY - 4.0f,
            8.0f,
            8.0f);

        return;
    }

    // ==========================================================
    // CALCULATE INDICATOR POSITION
    // ==========================================================

    const float usableWidth =
        radius * 1.44f;

    const float indicatorX =
        centreX
        - usableWidth * 0.5f
        + normalized * usableWidth;

    // ==========================================================
    // TUNING QUALITY
    // ==========================================================
    //
    // Determine how close the singer is to perfect pitch.
    //

    const double absoluteCents =
        std::abs(cents);

    const bool veryClose =
        absoluteCents <= 5.0;

    const bool close =
        absoluteCents <= 15.0;

    juce::Colour indicatorColour;

    if (veryClose)
    {
        // Excellent tuning.
        indicatorColour =
            juce::Colour(0xff55ff9a);
    }
    else if (close)
    {
        // Acceptable tuning.
        indicatorColour =
            accent;
    }
    else
    {
        // Clearly flat or sharp.
        indicatorColour =
            juce::Colour(0xffff5a36);
    }

    // ==========================================================
    // INDICATOR GLOW
    // ==========================================================

    const float glowRadius =
        veryClose ? 10.0f : 7.0f;

    g.setColour(
        indicatorColour.withAlpha(0.12f));

    g.fillEllipse(
        indicatorX - glowRadius,
        centreY - glowRadius,
        glowRadius * 2.0f,
        glowRadius * 2.0f);

    // ==========================================================
    // INDICATOR LINE
    // ==========================================================

    g.setColour(
        indicatorColour.withAlpha(0.75f));

    g.drawLine(
        indicatorX,
        centreY - radius * 0.35f,
        indicatorX,
        centreY + radius * 0.35f,
        2.0f);

    // ==========================================================
    // MAIN INDICATOR
    // ==========================================================

    g.setColour(
        indicatorColour);

    g.fillEllipse(
        indicatorX - 5.0f,
        centreY - 5.0f,
        10.0f,
        10.0f);

    // ==========================================================
    // CENTER TARGET
    // ==========================================================
    //
    // Keep the perfect-pitch target visible underneath the
    // moving indicator.
    //

    g.setColour(
        accent.withAlpha(0.35f));

    g.drawEllipse(
        centreX - 7.0f,
        centreY - 7.0f,
        14.0f,
        14.0f,
        1.5f);
}

//==============================================================================
// GLOBAL SECTION
//==============================================================================

void
OfforVocalProAudioProcessorEditor::drawGlobalSection(
    juce::Graphics& g,
    juce::Rectangle<int> bounds)
{
    g.setColour(
        panelColour);

    g.fillRoundedRectangle(
        bounds.toFloat(),
        9.0f);

    g.setColour(
        borderColour);

    g.drawRoundedRectangle(
        bounds.toFloat(),
        9.0f,
        1.0f);
}

//==============================================================================
// KEY
//==============================================================================

void
OfforVocalProAudioProcessorEditor::populateKeyComboBox()
{
    keyComboBox.clear();

    const juce::StringArray keys =
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
    };

    for (int i = 0; i < keys.size(); ++i)
    {
        keyComboBox.addItem(
            keys[i],
            i + 1);
    }
}

//==============================================================================
// SCALE
//==============================================================================

void
OfforVocalProAudioProcessorEditor::populateScaleComboBox()
{
    scaleComboBox.clear();

    scaleComboBox.addItem(
        "Chromatic",
        1);

    scaleComboBox.addItem(
        "Major",
        2);

    scaleComboBox.addItem(
        "Minor",
        3);
}

//==============================================================================
// MODE
//==============================================================================

void
OfforVocalProAudioProcessorEditor::populateModeComboBox()
{
    modeComboBox.clear();

    modeComboBox.addItem(
        "Natural",
        1);

    modeComboBox.addItem(
        "Modern",
        2);

    modeComboBox.addItem(
        "Hard Tune",
        3);
}

//==============================================================================
// HARMONY
//==============================================================================

void
OfforVocalProAudioProcessorEditor::
populateHarmonyComboBoxes()
{
    const juce::StringArray choices =
    {
        "Off",
        "3rd Up",
        "5th Up",
        "Octave Up",
        "3rd Down",
        "5th Down",
        "Octave Down"
    };

    auto populate =
        [&choices](juce::ComboBox& box)
        {
            box.clear();

            for (int i = 0;
                 i < choices.size();
                 ++i)
            {
                box.addItem(
                    choices[i],
                    i + 1);
            }
        };

    populate(
        harmonyVoice1ComboBox);

    populate(
        harmonyVoice2ComboBox);

    populate(
        harmonyVoice3ComboBox);

    populate(
        harmonyVoice4ComboBox);
}

//==============================================================================
// CREATIVE FX
//==============================================================================

void
OfforVocalProAudioProcessorEditor::
populateCreativeFxComboBox()
{
    creativeFxTypeComboBox.clear();

    const juce::StringArray choices =
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
    };

    for (int i = 0;
         i < choices.size();
         ++i)
    {
        creativeFxTypeComboBox.addItem(
            choices[i],
            i + 1);
    }
}

//==============================================================================
// SPACE
//==============================================================================

void
OfforVocalProAudioProcessorEditor::
populateSpaceComboBox()
{
    spaceTypeComboBox.clear();

    const juce::StringArray choices =
    {
        "Off",
        "Room",
        "Plate",
        "Hall",
        "Dark",
        "Air",
        "Dream"
    };

    for (int i = 0;
         i < choices.size();
         ++i)
    {
        spaceTypeComboBox.addItem(
            choices[i],
            i + 1);
    }
}



//==============================================================================
// TIMER
//==============================================================================

void
OfforVocalProAudioProcessorEditor::
timerCallback()
{
       // ==========================================================
    // LICENSE STATUS
    // ==========================================================
    //
    // Always check licensing first.
    //
    // This allows the editor to remain open after the trial is
    // exhausted while displaying the activation screen.
    //

    updateLicenseOverlay();

    // ==========================================================
    // LEVEL METERS
    // ==========================================================
    //
    // Always update these first. The tuner may have no pitch,
    // but the audio signal can still be present.
    //

    inputMeter.setLevelDecibels(
        audioProcessor.getInputLevelDb());

    outputMeter.setLevelDecibels(
        audioProcessor.getOutputLevelDb());

    // ==========================================================
    // TUNER
    // ==========================================================

    if (!audioProcessor.hasDetectedPitch())
    {
        detectedNoteLabel.setText(
            "--",
            juce::dontSendNotification);

        detectedFrequencyLabel.setText(
            "-- Hz",
            juce::dontSendNotification);

        detectedCentsLabel.setText(
            "-- cents",
            juce::dontSendNotification);

        detectedConfidenceLabel.setText(
            "NO SIGNAL",
            juce::dontSendNotification);

        tunerStatusLabel.setText(
            "READY",
            juce::dontSendNotification);

        tunerStatusLabel.setColour(
            juce::Label::textColourId,
            mutedColour);

        return;
    }

    const double midi =
        audioProcessor.getDetectedMidiNote();

    const double frequency =
        audioProcessor.getDetectedFrequency();

    const double cents =
        audioProcessor.getDetectedCents();

    const double confidence =
        audioProcessor.getPitchConfidence();

    static const char* noteNames[] =
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
    };

    const int roundedMidi =
        juce::jlimit(
            0,
            127,
            static_cast<int>(
                std::round(midi)));

    const int noteIndex =
        roundedMidi % 12;

    const int octave =
        (roundedMidi / 12) - 1;

    const juce::String noteName =
        juce::String(
            noteNames[noteIndex])
        + juce::String(octave);

    detectedNoteLabel.setText(
        noteName,
        juce::dontSendNotification);

    detectedFrequencyLabel.setText(
        juce::String(
            frequency,
            1)
        + " Hz",
        juce::dontSendNotification);

    const juce::String centsText =
        (cents >= 0.0
            ? "+"
            : "")
        + juce::String(
            cents,
            1)
        + " cents";

    detectedCentsLabel.setText(
        centsText,
        juce::dontSendNotification);

    detectedConfidenceLabel.setText(
        "CONFIDENCE "
        + juce::String(
            confidence * 100.0,
            0)
        + "%",
        juce::dontSendNotification);

    tunerStatusLabel.setText(
        "TRACKING",
        juce::dontSendNotification);

    tunerStatusLabel.setColour(
        juce::Label::textColourId,
        accentColour);

    repaint();
}



//==============================================================================
// UPDATE LICENSE OVERLAY
//==============================================================================
//
// This function determines whether the trial-complete screen
// should be visible.
//
// IMPORTANT:
//
// We do NOT use only getFreeUsesRemaining() here because that
// value has local fallback behaviour.
//
// We specifically wait until the licensing session has started
// and the server has reported the actual server-side usage.
//
//==============================================================================



// ==========================================================
// UPDATE LICENSE OVERLAY
// ==========================================================

void OfforVocalProAudioProcessorEditor::updateLicenseOverlay()
{
    if (licenseOverlay == nullptr)
        return;

    // ==========================================================
    // LICENSE ALREADY ACTIVATED
    // ==========================================================
    if (audioProcessor.isLicenseActivated())
    {
        licenseOverlay->setVisible(false);
        return;
    }

    // ==========================================================
    // LICENSE SESSION HAS NOT FINISHED INITIALIZING
    // ==========================================================
    if (!audioProcessor.isLicenseSessionStarted())
    {
        licenseOverlay->setVisible(false);
        return;
    }

    // ==========================================================
    // CHECK SERVER TRIAL USAGE
    // ==========================================================
    const int uses =
        audioProcessor.getServerFreeUses();

    const int limit =
        audioProcessor.getServerFreeUsesLimit();

    // ==========================================================
    // TRIAL EXHAUSTED
    // ==========================================================
    if (limit > 0 && uses >= limit)
    {
        // Make sure it covers the entire editor.
        licenseOverlay->setBounds(getLocalBounds());

        // Show it.
        licenseOverlay->setVisible(true);

        // Put it above every other component.
        licenseOverlay->toFront(true);

        // Force the overlay to redraw.
        licenseOverlay->repaint();

        return;
    }

    // ==========================================================
    // TRIAL STILL AVAILABLE
    // ==========================================================
    licenseOverlay->setVisible(false);
}



//==============================================================================
// BEGIN LICENSE ACTIVATION
//==============================================================================

void
OfforVocalProAudioProcessorEditor::
beginLicenseActivation()
{
    if (licenseOverlay == nullptr)
        return;

    const juce::String key =
        licenseOverlay->getLicenseKey();

    if (key.isEmpty())
    {
        licenseOverlay->setActivationState(
            false,
            "Please enter your license key.");

        return;
    }

    // ----------------------------------------------------------
    // Prevent two activation requests at the same time.
    // ----------------------------------------------------------

    if (licenseActivationThread != nullptr &&
        licenseActivationThread->isThreadRunning())
    {
        return;
    }

    // ----------------------------------------------------------
    // Show activation state immediately.
    // ----------------------------------------------------------

    licenseOverlay->setActivationState(
        true);

    // ----------------------------------------------------------
    // Network activation happens away from the message thread.
    // ----------------------------------------------------------

    licenseActivationThread =
        std::make_unique<LicenseActivationThread>(
            *this,
            key);

    licenseActivationThread->startThread();
}


//==============================================================================
// FINISH LICENSE ACTIVATION
//==============================================================================

void
OfforVocalProAudioProcessorEditor::
finishLicenseActivation(
    bool success,
    const juce::String& message)
{
    // ----------------------------------------------------------
    // Thread has completed.
    // ----------------------------------------------------------

    if (licenseActivationThread != nullptr)
    {
        licenseActivationThread->stopThread(
            0);

        licenseActivationThread.reset();
    }

    // ----------------------------------------------------------
    // Successful activation.
    // ----------------------------------------------------------

    if (success)
    {
        if (licenseOverlay != nullptr)
        {
            licenseOverlay->setVisible(false);
        }

        repaint();

        return;
    }

    // ----------------------------------------------------------
    // Failed activation.
    // ----------------------------------------------------------

    if (licenseOverlay != nullptr)
    {
        licenseOverlay->setActivationState(
            false,
            message);
    }
}

//==============================================================================
// OPEN LICENSE PURCHASE PAGE
//==============================================================================

void
OfforVocalProAudioProcessorEditor::
openLicensePurchasePage()
{
    // ==========================================================
    // OFFOR VOCAL PRO PURCHASE URL
    // ==========================================================
    //
    // TODO:
    // Replace this with the FINAL Offor Vocal Pro checkout URL
    // once your product checkout page is ready.
    //
    // Do not forget to replace this before release.
    //
    const juce::URL purchaseURL(
        "https://ko-fi.com/s/0bda33ea5c");

    purchaseURL.launchInDefaultBrowser();
}



//==============================================================================
// SAVE PRESET TO FILE
//==============================================================================
//
// SAVE creates a real OFFOR Vocal Pro preset on disk.
//
// The complete APVTS state is stored, meaning the preset contains
// all plugin parameters rather than only the currently visible
// module settings.
//
// A/B is NOT affected by SAVE.
//
//==============================================================================

void
OfforVocalProAudioProcessorEditor::savePresetToFile()
{
    // ----------------------------------------------------------
    // Create the file chooser.
    // ----------------------------------------------------------
    //
    // Keep the chooser alive as a member of the editor because
    // launchAsync() requires the FileChooser to remain alive
    // until the user finishes selecting a location.
    //

    const auto presetFolder =
        juce::File::getSpecialLocation(
            juce::File::userDocumentsDirectory)
        .getChildFile(
            "OFFOR Vocal Pro");

    // Create the folder if it does not already exist.
    presetFolder.createDirectory();

    presetFileChooser =
        std::make_unique<juce::FileChooser>(
            "Save OFFOR Vocal Pro Preset",
            presetFolder
                .getChildFile(
                    "My Vocal Preset.offorvocalpreset"),
            "*.offorvocalpreset");

    presetFileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::warnAboutOverwriting,

        [this](const juce::FileChooser& chooser)
        {
            const juce::File file =
                chooser.getResult();

            if (file == juce::File{})
                return;

            // --------------------------------------------------
            // Make sure the correct preset extension exists.
            // --------------------------------------------------

            juce::File presetFile = file;

            if (!presetFile.hasFileExtension(
                    "offorvocalpreset"))
            {
                presetFile =
                    presetFile.withFileExtension(
                        "offorvocalpreset");
            }

            // --------------------------------------------------
            // Copy the COMPLETE plugin state.
            // --------------------------------------------------

            const juce::ValueTree state =
                audioProcessor.apvts.copyState();

            // Convert the ValueTree to XML.
            std::unique_ptr<juce::XmlElement> xml =
                state.createXml();

            if (xml == nullptr)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "OFFOR VOCAL PRO",
                    "Could not create the preset data.",
                    "OK");

                return;
            }

            // --------------------------------------------------
            // Add preset information.
            // --------------------------------------------------

            xml->setAttribute(
                "plugin",
                "OFFOR VOCAL PRO");

            xml->setAttribute(
                "presetVersion",
                "1.0");

            // --------------------------------------------------
            // Write the XML to disk.
            // --------------------------------------------------

            if (!xml->writeTo(
                    presetFile))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "OFFOR VOCAL PRO",
                    "Could not save the preset file.",
                    "OK");

                return;
            }

            // --------------------------------------------------
            // SAVE successful.
            // --------------------------------------------------

            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "OFFOR VOCAL PRO",
                "Preset saved successfully:\n\n"
                + presetFile.getFileName(),
                "OK");
        });
}


//==============================================================================
// LOAD PRESET FROM FILE
//==============================================================================
//
// Loads a previously saved OFFOR Vocal Pro preset.
//
// The complete APVTS state is restored.
//
//==============================================================================

void
OfforVocalProAudioProcessorEditor::loadPresetFromFile(
    const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    // ----------------------------------------------------------
    // Read the XML file.
    // ----------------------------------------------------------

    auto xml =
        juce::XmlDocument::parse(file);

    if (xml == nullptr)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "OFFOR VOCAL PRO",
            "The selected preset file is invalid.",
            "OK");

        return;
    }

    // ----------------------------------------------------------
    // Convert XML back into a ValueTree.
    // ----------------------------------------------------------

    const juce::ValueTree loadedState =
        juce::ValueTree::fromXml(*xml);

    if (!loadedState.isValid())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "OFFOR VOCAL PRO",
            "The selected preset could not be loaded.",
            "OK");

        return;
    }

    // ----------------------------------------------------------
    // Restore the complete APVTS state.
    // ----------------------------------------------------------

    audioProcessor.apvts.replaceState(
        loadedState);

    // ----------------------------------------------------------
    // Repaint the interface so all controls immediately
    // reflect the loaded preset.
    // ----------------------------------------------------------

    repaint();

    // ----------------------------------------------------------
    // The loaded preset becomes the current A state.
    //
    // B remains untouched.
    // ----------------------------------------------------------

    stateA =
        audioProcessor.apvts.copyState();

    isAActive = true;

    updateABButtonStates();
}