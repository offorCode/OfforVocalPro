#pragma once

#include <JuceHeader.h>

//==============================================================================
// OFFOR VOCAL PRO - PROFESSIONAL LEVEL METER
//
// This component is intentionally independent from the audio processor.
//
// Later, PluginEditor will simply do:
//
//     inputMeter.setLevel(...);
//     outputMeter.setLevel(...);
//
// The meter itself does not know whether it is INPUT or OUTPUT.
//==============================================================================

class LevelMeter : public juce::Component
{
public:

    // ==========================================================
    // CONSTRUCTOR
    // ==========================================================

    LevelMeter();

    // ==========================================================
    // SETUP
    // ==========================================================

    // Set the text displayed above the meter.
    //
    // Example:
    //     setLabel("INPUT");
    //     setLabel("OUTPUT");
    //
    void setLabel(const juce::String& newLabel);

    // ==========================================================
    // LEVEL
    // ==========================================================

    // Set the current linear audio level.
    //
    // Expected range:
    //     0.0f = silence
    //     1.0f = 0 dBFS
    //
    // The component converts this to dB internally.
    void setLevel(float newLevel);

    // Set the level directly in decibels.
    //
    // Example:
    //     setLevelDecibels(-6.0f);
    //
    void setLevelDecibels(float newLevelDb);

    // ==========================================================
    // PEAK
    // ==========================================================

    // Set the peak level directly in decibels.
    //
    // This is useful later when the processor provides
    // a peak-hold value.
    void setPeakDecibels(float newPeakDb);

    // Reset the peak indicator.
    void resetPeak();

    // ==========================================================
    // RANGE
    // ==========================================================

    // Change the minimum displayed dB value.
    //
    // Default:
    //     -60 dB
    //
    void setMinimumDecibels(float newMinimumDb);

    // Change the maximum displayed dB value.
    //
    // Default:
    //     0 dB
    //
    void setMaximumDecibels(float newMaximumDb);

    // ==========================================================
    // PAINT
    // ==========================================================

    void paint(juce::Graphics& g) override;

    // ==========================================================
    // RESIZE
    // ==========================================================

    void resized() override;

private:

    // ==========================================================
    // DISPLAY LABEL
    // ==========================================================

    juce::String label = "LEVEL";

    // ==========================================================
    // AUDIO LEVEL
    // ==========================================================

    // Target level supplied by the processor/editor.
    float targetLevelDb = -60.0f;

    // Smoothed visual level.
    float displayedLevelDb = -60.0f;

    // Peak-hold value.
    float peakLevelDb = -60.0f;

    // Time that the peak indicator has been held.
    int peakHoldCounter = 0;

    // ==========================================================
    // RANGE
    // ==========================================================

    float minimumDecibels = -60.0f;
    float maximumDecibels = 0.0f;

    // ==========================================================
    // METER SETTINGS
    // ==========================================================

    static constexpr int numberOfSegments = 24;

    static constexpr float segmentGap = 2.0f;

    static constexpr float smoothingUp = 0.35f;

    static constexpr float smoothingDown = 0.12f;

    static constexpr int peakHoldFrames = 45;

    // ==========================================================
    // INTERNAL HELPERS
    // ==========================================================

    float normaliseLevel(float decibels) const;

    int getActiveSegmentCount() const;

    juce::Rectangle<float> getMeterBounds() const;

    void updateSmoothing();

    void drawMeterBackground(
        juce::Graphics& g,
        juce::Rectangle<float> bounds);

    void drawSegments(
        juce::Graphics& g,
        juce::Rectangle<float> bounds);

    void drawPeakIndicator(
        juce::Graphics& g,
        juce::Rectangle<float> bounds);

    void drawDbValue(
        juce::Graphics& g);

    void drawScale(
        juce::Graphics& g,
        juce::Rectangle<float> bounds);

    // ==========================================================
    // COLOURS
    // ==========================================================

    static const juce::Colour backgroundColour;
    static const juce::Colour meterBackgroundColour;
    static const juce::Colour segmentOffColour;
    static const juce::Colour segmentLowColour;
    static const juce::Colour segmentMidColour;
    static const juce::Colour segmentHighColour;
    static const juce::Colour peakColour;
    static const juce::Colour textColour;
    static const juce::Colour mutedColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};