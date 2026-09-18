#pragma once

#include <JuceHeader.h>

//==============================================================================
// OFFOR VOCAL PRO - THEME MANAGER
//
// ThemeManager is the SINGLE SOURCE OF TRUTH for the entire plugin UI.
//
// All visual components should obtain their colours from:
//
//     ThemeManager::get().getColours()
//
// Built-in themes:
//
//     OFFOR Dark
//     Midnight Blue
//     Graphite
//     Purple Studio
//     Emerald
//     Crimson
//     Light
//     Custom
//
// IMPORTANT:
//
// ThemeManager inherits from juce::ChangeBroadcaster.
//
// This allows PluginEditor, SettingsPanel, ThemeColorPanel, meters,
// knobs, etc. to react when the user changes a theme or custom colour.
//
//==============================================================================

class ThemeManager
    : public juce::ChangeBroadcaster
{
public:

    //==========================================================================
    // THEME COLOURS
    //==========================================================================

    struct Colours
    {
        juce::Colour background;
        juce::Colour panel;
        juce::Colour panel2;
        juce::Colour border;

        juce::Colour text;
        juce::Colour muted;

        juce::Colour accent;
        juce::Colour accentDark;

        juce::Colour display;

        juce::Colour knob;
        juce::Colour knobHighlight;

        juce::Colour meter;
        juce::Colour meterBackground;

        juce::Colour success;
        juce::Colour warning;
    };

    //==========================================================================
    // SINGLETON
    //==========================================================================

    static ThemeManager& get();

    //==========================================================================
    // THEME
    //==========================================================================

    void setTheme(
        const juce::String& themeName);

    const juce::String& getThemeName() const;

    Colours getColours() const;

    static juce::StringArray getThemeNames();

    //==========================================================================
    // CUSTOM COLOURS
    //==========================================================================

    void setCustomColour(
        const juce::String& colourId,
        juce::Colour colour);

    juce::Colour getCustomColour(
        const juce::String& colourId) const;

    void resetCustomColours();


    void setCustomColourWithoutChangingTheme(
    const juce::String& colourId,
    juce::Colour colour);

    //==========================================================================
    // LEGACY CALLBACK
    //==========================================================================
    //
    // Kept for compatibility with existing code.
    //
    // New code should preferably use ChangeListener.
    //
    //==========================================================================

    std::function<void()> onThemeChanged;

private:

    ThemeManager();
    ~ThemeManager() override = default;

    //==========================================================================
    // BUILT-IN THEME FACTORIES
    //==========================================================================

    Colours createOFFORDarkTheme() const;
    Colours createMidnightBlueTheme() const;
    Colours createGraphiteTheme() const;
    Colours createPurpleStudioTheme() const;
    Colours createEmeraldTheme() const;
    Colours createCrimsonTheme() const;
    Colours createLightTheme() const;
    Colours createCustomTheme() const;

    //==========================================================================
    // DATA
    //==========================================================================

    Colours customColours;

    juce::String currentTheme = "OFFOR Dark";

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};