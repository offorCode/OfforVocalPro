#include "PluginEditor.h"
#include "BinaryData.h"

#include <cmath>

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
    // SETTINGS
    // ==========================================================

    settingsButton.setButtonText(
        "SETTINGS");

    settingsButton.setWantsKeyboardFocus(
        false);

    settingsButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    settingsButton.onClick =
        [this]
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "OFFOR Vocal Pro",
                "Settings panel will be available here.",
                "OK");
        };

    addAndMakeVisible(
        settingsButton);

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
    // START
    // ==========================================================

    selectModule(
        Module::tuner);

    startTimerHz(20);
}

//==============================================================================
// DESTRUCTOR
//==============================================================================

OfforVocalProAudioProcessorEditor::
~OfforVocalProAudioProcessorEditor()
{
    stopTimer();
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

    versionLabel.setBounds(
        w - 280,
        20,
        55,
        20);

    settingsButton.setBounds(
        w - 205,
        16,
        80,
        32);

    bypassButton.setBounds(
        w - 112,
        17,
        90,
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

    const int innerX = panelX + 24;
    const int innerW = panelW - 48;

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

    // doublerTitleLabel.setBounds(
    //     innerX,
    //     panelY + 14,
    //     240,
    //     24);

    const int dY =
        panelY + 115;

    const int dWidth =
        juce::jmax(
            85,
            (innerW - 48) / 5);

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

    for (int i = 0; i < 5; ++i)
    {
        const int x =
            innerX + i * (dWidth + 12);

        dLabels[i]->setBounds(
            x,
            dY,
            dWidth,
            18);

        dSliders[i]->setBounds(
            x,
            dY + 18,
            dWidth,
            105);
    }

    // ==========================================================
    // HARMONY
    // ==========================================================

    // harmonyTitleLabel.setBounds(
    //     innerX,
    //     panelY + 14,
    //     240,
    //     24);

    const int harmonyY =
        panelY + 100;

    const int harmonyRowHeight =
        50;

    const int harmonyLabelWidth =
        80;

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
            innerW - harmonyLabelWidth - 10,
            34);
    }

    harmonyMixLabel.setBounds(
        innerX,
        harmonyY + 220,
        120,
        18);

    harmonyMixSlider.setBounds(
        innerX,
        harmonyY + 240,
        160,
        90);

    // ==========================================================
    // CREATIVE FX
    // ==========================================================

    // creativeFxTitleLabel.setBounds(
    //     innerX,
    //     panelY + 14,
    //     240,
    //     24);

    creativeFxTypeLabel.setBounds(
        innerX,
        panelY + 100,
        100,
        18);

    creativeFxTypeComboBox.setBounds(
        innerX,
        panelY + 124,
        innerW,
        38);

    const int fxKnobY =
        panelY + 220;

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

    // spaceTitleLabel.setBounds(
    //     innerX,
    //     panelY + 14,
    //     240,
    //     24);

    spaceTypeLabel.setBounds(
        innerX,
        panelY + 100,
        80,
        18);

    spaceTypeComboBox.setBounds(
        innerX,
        panelY + 124,
        innerW,
        38);

    const int sY =
        panelY + 215;

    const int sWidth =
        juce::jmax(
            80,
            (innerW - 48) / 5);

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
            innerX + i * (sWidth + 12);

        sLabels[i]->setBounds(
            x,
            sY,
            sWidth,
            18);

        sSliders[i]->setBounds(
            x,
            sY + 18,
            sWidth,
            105);
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
// PITCH METER
//==============================================================================

void
OfforVocalProAudioProcessorEditor::drawPitchMeter(
    juce::Graphics& g,
    juce::Rectangle<int> bounds)
{
    const float centreX =
        bounds.getCentreX();

    const float centreY =
        static_cast<float>(
            bounds.getCentreY());

    const float radius =
        juce::jmin(
            bounds.getWidth() * 0.28f,
            bounds.getHeight() * 0.48f);

    g.setColour(
        displayColour);

    g.fillEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f);

    g.setColour(
        borderColour);

    g.drawEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    g.setColour(
        accentColour);

    g.fillEllipse(
        centreX - 4.0f,
        centreY - 4.0f,
        8.0f,
        8.0f);

    g.setColour(
        mutedColour);

    g.drawLine(
        centreX - radius * 0.72f,
        centreY,
        centreX + radius * 0.72f,
        centreY,
        1.0f);

    g.setColour(
        borderColour);

    for (int i = -4; i <= 4; ++i)
    {
        const float x =
            centreX
            + (static_cast<float>(i) / 4.0f)
              * radius * 0.72f;

        g.drawLine(
            x,
            centreY - 6.0f,
            x,
            centreY + 6.0f,
            1.0f);
    }
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