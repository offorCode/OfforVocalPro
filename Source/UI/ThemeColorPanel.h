#pragma once

#include <JuceHeader.h>
#include "ThemeManager.h"

//==============================================================================
// ThemeColorPanel
//
// Standalone colour-customisation panel for OFFOR Vocal Pro.
//
// The panel provides 15 editable interface colours:
//
// Background
// Panel
// Panel 2
// Border
// Text
// Muted Text
// Accent
// Accent Dark
// Display
// Knob
// Knob Highlight
// Meter
// Meter Background
// Success
// Warning
//
// Colour changes are written directly into ThemeManager.
//
//==============================================================================

class ThemeColorPanel : public juce::Component
{
public:

    //==========================================================================
    // Individual colour row
    //==========================================================================

    class ColourRow : public juce::Component
    {
    public:

        ColourRow(
            const juce::String& colourName,
            const juce::String& colourId);

        ~ColourRow() override = default;

        //======================================================================

        void paint(
            juce::Graphics& g) override;

        void resized() override;

        // Opens the JUCE colour picker.
        void openColourSelector();

        // Refreshes the colour preview.
        void updateDisplay();

    private:

        //======================================================================
        // Colour ID used by ThemeManager.
        //======================================================================

        juce::String id;

        //======================================================================
        // UI
        //======================================================================

        juce::Label nameLabel;

        juce::TextButton colourButton;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            ColourRow)
    };

    //==========================================================================

    ThemeColorPanel();

    ~ThemeColorPanel() override = default;

    //==========================================================================

    void paint(
        juce::Graphics& g) override;

    void resized() override;

    // Refresh the complete panel.
    void updateDisplay();

    //==========================================================================

    std::function<void()> onClose;

private:

    //==========================================================================
    // All colour rows.
    //
    // unique_ptr is used because the rows are dynamically created and their
    // lifetime is owned by ThemeColorPanel.
    //==========================================================================

    std::vector<std::unique_ptr<ColourRow>> colourRows;

    //==========================================================================

    juce::TextButton resetButton;
    juce::TextButton closeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ThemeColorPanel)
};
