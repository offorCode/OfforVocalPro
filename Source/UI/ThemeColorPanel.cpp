
#include "ThemeColorPanel.h"

//==============================================================================
// ThemeColorPanel::ColourRow
//==============================================================================

ThemeColorPanel::ColourRow::ColourRow(
    const juce::String& colourName,
    const juce::String& colourId)
    : id(colourId)
{
    //==========================================================================
    // Colour name
    //==========================================================================

    nameLabel.setText(
        colourName,
        juce::dontSendNotification);

    nameLabel.setColour(
        juce::Label::textColourId,
        ThemeManager::get().getColours().text);

    addAndMakeVisible(nameLabel);

    //==========================================================================
    // Colour preview button
    //==========================================================================

    colourButton.setButtonText("");

    colourButton.setTooltip(
        "Click to change " + colourName);

    colourButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    colourButton.onClick = [this]()
    {
        openColourSelector();
    };

    addAndMakeVisible(colourButton);
}

//==============================================================================

void ThemeColorPanel::ColourRow::paint(
    juce::Graphics& g)
{
    const auto colours =
        ThemeManager::get().getColours();

    //==========================================================================
    // Row background
    //==========================================================================

    g.setColour(colours.panel);

    g.fillRoundedRectangle(
        getLocalBounds().toFloat(),
        6.0f);

    //==========================================================================
    // Row border
    //==========================================================================

    g.setColour(colours.border);

    g.drawRoundedRectangle(
        getLocalBounds().toFloat().reduced(0.5f),
        6.0f,
        1.0f);

    //==========================================================================
    // Colour preview
    //==========================================================================

    const auto preview =
        colourButton.getBounds()
            .toFloat()
            .reduced(3.0f);

    g.setColour(
        ThemeManager::get().getCustomColour(id));

    g.fillRoundedRectangle(
        preview,
        5.0f);

    // Preview border
    g.setColour(colours.border);

    g.drawRoundedRectangle(
        preview,
        5.0f,
        1.0f);
}

//==============================================================================

void ThemeColorPanel::ColourRow::resized()
{
    auto area =
        getLocalBounds()
            .reduced(10, 5);

    // Name
    nameLabel.setBounds(
        area.removeFromLeft(170));

    area.removeFromLeft(8);

    // Colour preview
    colourButton.setBounds(
        area.removeFromLeft(70));
}

//==============================================================================

void ThemeColorPanel::ColourRow::updateDisplay()
{
    const auto colours =
        ThemeManager::get().getColours();

    nameLabel.setColour(
        juce::Label::textColourId,
        colours.text);

    repaint();
}

//==============================================================================
// SAFE COLOUR SELECTOR
//==============================================================================

void ThemeColorPanel::ColourRow::openColourSelector()
{
    //==========================================================================
    // This wrapper owns the ColourSelector.
    //
    // The wrapper is then transferred to CallOutBox.
    //
    // This prevents:
    //
    //   - dangling ColourSelector pointers
    //   - unique_ptr/reference conversion errors
    //   - ColourRow lifetime problems
    //
    // The ColourSelector itself uses JUCE's ChangeListener API.
    //==========================================================================

    class ColourSelectorWrapper
        : public juce::Component,
          private juce::ChangeListener
    {
    public:

        ColourSelectorWrapper(
            const juce::String& colourId,
            juce::Colour initialColour)
            : id(colourId)
        {
            //==================================================================
            // Configure selector
            //==================================================================

            selector.setCurrentColour(
                initialColour);

            selector.setColour(
                juce::ColourSelector::backgroundColourId,
                ThemeManager::get().getColours().panel);

            selector.setColour(
                juce::ColourSelector::labelTextColourId,
                ThemeManager::get().getColours().text);

            selector.addChangeListener(this);

            addAndMakeVisible(selector);

            setSize(420, 500);
        }

        ~ColourSelectorWrapper() override
        {
            selector.removeChangeListener(this);
        }

        //======================================================================

        void resized() override
        {
            selector.setBounds(
                getLocalBounds().reduced(8));
        }

    private:

        //======================================================================
        // ColourSelector changed
        //======================================================================

        void changeListenerCallback(
            juce::ChangeBroadcaster* source) override
        {
            if (source == &selector)
            {
                // Write the colour immediately to ThemeManager.
                ThemeManager::get().setCustomColour(
                    id,
                    selector.getCurrentColour());
            }
        }

        //======================================================================

        juce::String id;

        juce::ColourSelector selector;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            ColourSelectorWrapper)
    };

    //==========================================================================
    // Get current colour.
    //==========================================================================

    const auto currentColour =
        ThemeManager::get().getCustomColour(id);

    //==========================================================================
    // Create the popup component.
    //==========================================================================

    auto wrapper =
        std::make_unique<ColourSelectorWrapper>(
            id,
            currentColour);

    //==========================================================================
    // Display it using JUCE CallOutBox.
    //
    // CallOutBox takes ownership of wrapper.
    //==========================================================================

    juce::CallOutBox::launchAsynchronously(
        std::move(wrapper),
        colourButton.getScreenBounds(),
        nullptr);
}


//==============================================================================
// ThemeColorPanel
//==============================================================================

ThemeColorPanel::ThemeColorPanel()
{
    setOpaque(false);

    //==========================================================================
    // Colour names
    //==========================================================================

    const juce::StringArray names =
    {
        "Background",
        "Panel",
        "Panel 2",
        "Border",
        "Text",
        "Muted Text",
        "Accent",
        "Accent Dark",
        "Display",
        "Knob",
        "Knob Highlight",
        "Meter",
        "Meter Background",
        "Success",
        "Warning"
    };

    //==========================================================================
    // ThemeManager colour IDs
    //==========================================================================

    const juce::StringArray ids =
    {
        "background",
        "panel",
        "panel2",
        "border",
        "text",
        "muted",
        "accent",
        "accentDark",
        "display",
        "knob",
        "knobHighlight",
        "meter",
        "meterBackground",
        "success",
        "warning"
    };

    //==========================================================================
    // Create colour rows
    //==========================================================================

    for (int i = 0; i < names.size(); ++i)
    {
        auto row =
            std::make_unique<ColourRow>(
                names[i],
                ids[i]);

        addAndMakeVisible(*row);

        colourRows.push_back(
            std::move(row));
    }

    //==========================================================================
    // RESET CUSTOM COLOURS
    //==========================================================================

    resetButton.setButtonText(
        "RESET CUSTOM COLOURS");

    resetButton.onClick = [this]()
    {
        ThemeManager::get().resetCustomColours();

        updateDisplay();
    };

    addAndMakeVisible(resetButton);

    //==========================================================================
    // CLOSE
    //==========================================================================

    closeButton.setButtonText(
        "CLOSE");

    closeButton.onClick = [this]()
    {
        if (onClose)
            onClose();
    };

    addAndMakeVisible(closeButton);
}

//==============================================================================

void ThemeColorPanel::paint(
    juce::Graphics& g)
{
    const auto colours =
        ThemeManager::get().getColours();

    //==========================================================================
    // Main panel
    //==========================================================================

    g.setColour(colours.panel);

    g.fillRoundedRectangle(
        getLocalBounds().toFloat(),
        12.0f);

    //==========================================================================
    // Border
    //==========================================================================

    g.setColour(colours.border);

    g.drawRoundedRectangle(
        getLocalBounds().toFloat().reduced(0.5f),
        12.0f,
        1.0f);

    //==========================================================================
    // Title
    //==========================================================================

    g.setColour(colours.text);

    g.setFont(
        juce::Font(
            18.0f,
            juce::Font::bold));

    g.drawText(
        "CUSTOMIZE COLOURS",
        20,
        15,
        getWidth() - 40,
        25,
        juce::Justification::centredLeft);

    //==========================================================================
    // Subtitle
    //==========================================================================

    g.setColour(colours.muted);

    g.setFont(
        juce::Font(12.0f));

    g.drawText(
        "Create your own OFFOR Vocal Pro interface theme.",
        20,
        42,
        getWidth() - 40,
        20,
        juce::Justification::centredLeft);
}

//==============================================================================

void ThemeColorPanel::resized()
{
    auto area =
        getLocalBounds()
            .reduced(20);

    //==========================================================================
    // Header
    //==========================================================================

    area.removeFromTop(65);

    //==========================================================================
    // Bottom buttons
    //==========================================================================

    auto bottom =
        area.removeFromBottom(42);

    closeButton.setBounds(
        bottom.removeFromRight(100));

    bottom.removeFromRight(10);

    resetButton.setBounds(
        bottom.removeFromRight(190));

    //==========================================================================
    // Colour rows
    //==========================================================================

    constexpr int rowHeight = 42;
    constexpr int rowGap = 7;

    for (auto& row : colourRows)
    {
        auto rowArea =
            area.removeFromTop(rowHeight);

        row->setBounds(rowArea);

        area.removeFromTop(rowGap);
    }
}

//==============================================================================

void ThemeColorPanel::updateDisplay()
{
    //==========================================================================
    // Refresh panel
    //==========================================================================

    repaint();

    //==========================================================================
    // Refresh every colour row
    //==========================================================================

    for (auto& row : colourRows)
        row->updateDisplay();
}