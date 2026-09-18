#include "ThemeManager.h"

//==============================================================================
// SINGLETON
//==============================================================================

ThemeManager& ThemeManager::get()
{
    static ThemeManager instance;
    return instance;
}

//==============================================================================
// CONSTRUCTOR
//==============================================================================

ThemeManager::ThemeManager()
{
    // Custom colours initially start from the OFFOR Dark palette.
    customColours =
        createOFFORDarkTheme();
}

//==============================================================================
// SET THEME
//==============================================================================

void ThemeManager::setTheme(
    const juce::String& themeName)
{
    const auto themeNames =
        getThemeNames();

    if (!themeNames.contains(themeName))
        return;

    if (currentTheme == themeName)
    {
        // Still repaint listeners when explicitly selecting the
        // same theme. This is useful after resetting colours.
        sendChangeMessage();

        if (onThemeChanged)
            onThemeChanged();

        return;
    }

    currentTheme =
        themeName;

    //==========================================================================
    // IMPORTANT
    //==========================================================================
    //
    // Notify every registered UI component.
    //
    // PluginEditor
    // SettingsPanel
    // ThemeColorPanel
    // LevelMeter
    // etc.
    //
    // can now update themselves automatically.
    //
    //==========================================================================

    sendChangeMessage();

    if (onThemeChanged)
        onThemeChanged();
}

//==============================================================================
// GET CURRENT THEME NAME
//==============================================================================

const juce::String&
ThemeManager::getThemeName() const
{
    return currentTheme;
}

//==============================================================================
// GET CURRENT COLOURS
//==============================================================================

ThemeManager::Colours
ThemeManager::getColours() const
{
    if (currentTheme == "OFFOR Dark")
        return createOFFORDarkTheme();

    if (currentTheme == "Midnight Blue")
        return createMidnightBlueTheme();

    if (currentTheme == "Graphite")
        return createGraphiteTheme();

    if (currentTheme == "Purple Studio")
        return createPurpleStudioTheme();

    if (currentTheme == "Emerald")
        return createEmeraldTheme();

    if (currentTheme == "Crimson")
        return createCrimsonTheme();

    if (currentTheme == "Light")
        return createLightTheme();

    if (currentTheme == "Custom")
        return createCustomTheme();

    // Safety fallback.
    return createOFFORDarkTheme();
}

//==============================================================================
// THEME NAMES
//==============================================================================

juce::StringArray
ThemeManager::getThemeNames()
{
    return
    {
        "OFFOR Dark",
        "Midnight Blue",
        "Graphite",
        "Purple Studio",
        "Emerald",
        "Crimson",
        "Light",
        "Custom"
    };
}

//==============================================================================
// SET CUSTOM COLOUR
//==============================================================================

void ThemeManager::setCustomColour(
    const juce::String& colourId,
    juce::Colour colour)
{
    //==========================================================================
    // CUSTOM COLOUR MAP
    //==========================================================================

    if (colourId == "background")
        customColours.background = colour;

    else if (colourId == "panel")
        customColours.panel = colour;

    else if (colourId == "panel2")
        customColours.panel2 = colour;

    else if (colourId == "border")
        customColours.border = colour;

    else if (colourId == "text")
        customColours.text = colour;

    else if (colourId == "muted")
        customColours.muted = colour;

    else if (colourId == "accent")
        customColours.accent = colour;

    else if (colourId == "accentDark")
        customColours.accentDark = colour;

    else if (colourId == "display")
        customColours.display = colour;

    else if (colourId == "knob")
        customColours.knob = colour;

    else if (colourId == "knobHighlight")
        customColours.knobHighlight = colour;

    else if (colourId == "meter")
        customColours.meter = colour;

    else if (colourId == "meterBackground")
        customColours.meterBackground = colour;

    else if (colourId == "success")
        customColours.success = colour;

    else if (colourId == "warning")
        customColours.warning = colour;

    else
        return;

    //==========================================================================
    // A CUSTOM COLOUR CHANGE AUTOMATICALLY MAKES THE ACTIVE THEME "Custom".
    //==========================================================================

    currentTheme =
        "Custom";

    //==========================================================================
    // NOTIFY ALL UI COMPONENTS IMMEDIATELY.
    //==========================================================================

    sendChangeMessage();

    if (onThemeChanged)
        onThemeChanged();
}

//==============================================================================
// GET CUSTOM COLOUR
//==============================================================================

juce::Colour
ThemeManager::getCustomColour(
    const juce::String& colourId) const
{
    if (colourId == "background")
        return customColours.background;

    if (colourId == "panel")
        return customColours.panel;

    if (colourId == "panel2")
        return customColours.panel2;

    if (colourId == "border")
        return customColours.border;

    if (colourId == "text")
        return customColours.text;

    if (colourId == "muted")
        return customColours.muted;

    if (colourId == "accent")
        return customColours.accent;

    if (colourId == "accentDark")
        return customColours.accentDark;

    if (colourId == "display")
        return customColours.display;

    if (colourId == "knob")
        return customColours.knob;

    if (colourId == "knobHighlight")
        return customColours.knobHighlight;

    if (colourId == "meter")
        return customColours.meter;

    if (colourId == "meterBackground")
        return customColours.meterBackground;

    if (colourId == "success")
        return customColours.success;

    if (colourId == "warning")
        return customColours.warning;

    // Unknown colour ID.
    return juce::Colours::transparentBlack;
}

//==============================================================================
// RESET CUSTOM COLOURS
//==============================================================================

void ThemeManager::resetCustomColours()
{
    customColours =
        createOFFORDarkTheme();

    currentTheme =
        "Custom";

    sendChangeMessage();

    if (onThemeChanged)
        onThemeChanged();
}


// ==========================================================
// SET CUSTOM COLOUR WITHOUT CHANGING THEME
// ==========================================================
//
// Used when restoring plugin state.
//
// Unlike setCustomColour(), this does NOT automatically
// switch the current theme to "Custom".
//
// ==========================================================

void ThemeManager::setCustomColourWithoutChangingTheme(
    const juce::String& colourId,
    juce::Colour colour)
{
    if (colourId == "background")
        customColours.background = colour;

    else if (colourId == "panel")
        customColours.panel = colour;

    else if (colourId == "panel2")
        customColours.panel2 = colour;

    else if (colourId == "border")
        customColours.border = colour;

    else if (colourId == "text")
        customColours.text = colour;

    else if (colourId == "muted")
        customColours.muted = colour;

    else if (colourId == "accent")
        customColours.accent = colour;

    else if (colourId == "accentDark")
        customColours.accentDark = colour;

    else if (colourId == "display")
        customColours.display = colour;

    else if (colourId == "knob")
        customColours.knob = colour;

    else if (colourId == "knobHighlight")
        customColours.knobHighlight = colour;

    else if (colourId == "meter")
        customColours.meter = colour;

    else if (colourId == "meterBackground")
        customColours.meterBackground = colour;

    else if (colourId == "success")
        customColours.success = colour;

    else if (colourId == "warning")
        customColours.warning = colour;
}

//==============================================================================
// OFFOR DARK
//==============================================================================

ThemeManager::Colours
ThemeManager::createOFFORDarkTheme() const
{
    Colours c;

    c.background       = juce::Colour(0xff111216);
    c.panel            = juce::Colour(0xff17181d);
    c.panel2           = juce::Colour(0xff1d1f25);
    c.border           = juce::Colour(0xff30323a);

    c.text             = juce::Colour(0xfff2f2f4);
    c.muted            = juce::Colour(0xff858791);

    c.accent           = juce::Colour(0xffdf513e);
    c.accentDark       = juce::Colour(0xffa93629);

    c.display          = juce::Colour(0xff0c0e11);

    c.knob             = juce::Colour(0xff252832);
    c.knobHighlight    = juce::Colour(0xff555b68);

    c.meter            = juce::Colour(0xffdf513e);
    c.meterBackground  = juce::Colour(0xff090a0d);

    c.success          = juce::Colour(0xff65c98a);
    c.warning          = juce::Colour(0xffe6b85c);

    return c;
}

//==============================================================================
// MIDNIGHT BLUE
//==============================================================================

ThemeManager::Colours
ThemeManager::createMidnightBlueTheme() const
{
    Colours c;

    c.background       = juce::Colour(0xff0b1018);
    c.panel            = juce::Colour(0xff111925);
    c.panel2           = juce::Colour(0xff182333);
    c.border           = juce::Colour(0xff2a3a4f);

    c.text             = juce::Colour(0xffedf4ff);
    c.muted            = juce::Colour(0xff7f91a8);

    c.accent           = juce::Colour(0xff4d9cff);
    c.accentDark       = juce::Colour(0xff2869b3);

    c.display          = juce::Colour(0xff080d14);

    c.knob             = juce::Colour(0xff1d2b3e);
    c.knobHighlight    = juce::Colour(0xff526b87);

    c.meter            = juce::Colour(0xff4d9cff);
    c.meterBackground  = juce::Colour(0xff080c12);

    c.success          = juce::Colour(0xff62d39a);
    c.warning          = juce::Colour(0xffe6bb65);

    return c;
}

//==============================================================================
// GRAPHITE
//==============================================================================

ThemeManager::Colours
ThemeManager::createGraphiteTheme() const
{
    Colours c;

    c.background       = juce::Colour(0xff101112);
    c.panel            = juce::Colour(0xff191a1c);
    c.panel2           = juce::Colour(0xff242629);
    c.border           = juce::Colour(0xff3a3d41);

    c.text             = juce::Colour(0xfff0f0f0);
    c.muted            = juce::Colour(0xff92969c);

    c.accent           = juce::Colour(0xffb7bec7);
    c.accentDark       = juce::Colour(0xff747b84);

    c.display          = juce::Colour(0xff0b0c0d);

    c.knob             = juce::Colour(0xff292c30);
    c.knobHighlight    = juce::Colour(0xff626870);

    c.meter            = juce::Colour(0xffc3cad2);
    c.meterBackground  = juce::Colour(0xff0a0b0c);

    c.success          = juce::Colour(0xff70c999);
    c.warning          = juce::Colour(0xffd9b86a);

    return c;
}

//==============================================================================
// PURPLE STUDIO
//==============================================================================

ThemeManager::Colours
ThemeManager::createPurpleStudioTheme() const
{
    Colours c;

    c.background       = juce::Colour(0xff100d17);
    c.panel            = juce::Colour(0xff181321);
    c.panel2           = juce::Colour(0xff241a31);
    c.border           = juce::Colour(0xff413052);

    c.text             = juce::Colour(0xfff4efff);
    c.muted            = juce::Colour(0xff9a8cae);

    c.accent           = juce::Colour(0xffa86cff);
    c.accentDark       = juce::Colour(0xff7043b5);

    c.display          = juce::Colour(0xff0b0810);

    c.knob             = juce::Colour(0xff2a2038);
    c.knobHighlight    = juce::Colour(0xff75599a);

    c.meter            = juce::Colour(0xffa86cff);
    c.meterBackground  = juce::Colour(0xff09070d);

    c.success          = juce::Colour(0xff6dd29b);
    c.warning          = juce::Colour(0xffe2b963);

    return c;
}

//==============================================================================
// EMERALD
//==============================================================================

ThemeManager::Colours
ThemeManager::createEmeraldTheme() const
{
    Colours c;

    c.background       = juce::Colour(0xff0a1210);
    c.panel            = juce::Colour(0xff101c18);
    c.panel2           = juce::Colour(0xff172822);
    c.border           = juce::Colour(0xff294238);

    c.text             = juce::Colour(0xffedf8f3);
    c.muted            = juce::Colour(0xff829b90);

    c.accent           = juce::Colour(0xff32c98b);
    c.accentDark       = juce::Colour(0xff20835e);

    c.display          = juce::Colour(0xff07100d);

    c.knob             = juce::Colour(0xff1c332b);
    c.knobHighlight    = juce::Colour(0xff548b75);

    c.meter            = juce::Colour(0xff32c98b);
    c.meterBackground  = juce::Colour(0xff07100d);

    c.success          = juce::Colour(0xff65d69c);
    c.warning          = juce::Colour(0xffdfba67);

    return c;
}

//==============================================================================
// CRIMSON
//==============================================================================

ThemeManager::Colours
ThemeManager::createCrimsonTheme() const
{
    Colours c;

    c.background       = juce::Colour(0xff150b0d);
    c.panel            = juce::Colour(0xff201114);
    c.panel2           = juce::Colour(0xff2d181c);
    c.border           = juce::Colour(0xff4a292e);

    c.text             = juce::Colour(0xfffff0f1);
    c.muted            = juce::Colour(0xffa88b90);

    c.accent           = juce::Colour(0xffef4d68);
    c.accentDark       = juce::Colour(0xffa42e43);

    c.display          = juce::Colour(0xff0d0709);

    c.knob             = juce::Colour(0xff351b21);
    c.knobHighlight    = juce::Colour(0xff8e5360);

    c.meter            = juce::Colour(0xffef4d68);
    c.meterBackground  = juce::Colour(0xff0c0709);

    c.success          = juce::Colour(0xff65cf93);
    c.warning          = juce::Colour(0xffe3b55f);

    return c;
}

//==============================================================================
// LIGHT
//==============================================================================

ThemeManager::Colours
ThemeManager::createLightTheme() const
{
    Colours c;

    c.background       = juce::Colour(0xffe7e9ed);
    c.panel            = juce::Colour(0xfff5f6f8);
    c.panel2           = juce::Colour(0xffe1e4e9);
    c.border           = juce::Colour(0xffc3c7cf);

    c.text             = juce::Colour(0xff202329);
    c.muted            = juce::Colour(0xff6d727c);

    c.accent           = juce::Colour(0xffd94d3b);
    c.accentDark       = juce::Colour(0xffa73529);

    c.display          = juce::Colour(0xffd9dde3);

    c.knob             = juce::Colour(0xffd0d4da);
    c.knobHighlight    = juce::Colour(0xff8d949e);

    c.meter            = juce::Colour(0xffd94d3b);
    c.meterBackground  = juce::Colour(0xffc5c9cf);

    c.success          = juce::Colour(0xff329866);
    c.warning          = juce::Colour(0xffa47719);

    return c;
}

//==============================================================================
// CUSTOM
//==============================================================================

ThemeManager::Colours
ThemeManager::createCustomTheme() const
{
    return customColours;
}