#include "LevelMeter.h"

//==============================================================================
// COLOURS
//==============================================================================

const juce::Colour LevelMeter::backgroundColour =
    juce::Colour(0xff101116);

const juce::Colour LevelMeter::meterBackgroundColour =
    juce::Colour(0xff171920);

const juce::Colour LevelMeter::segmentOffColour =
    juce::Colour(0xff292c34);

const juce::Colour LevelMeter::segmentLowColour =
    juce::Colour(0xff5fd35f);

const juce::Colour LevelMeter::segmentMidColour =
    juce::Colour(0xffd8c84a);

const juce::Colour LevelMeter::segmentHighColour =
    juce::Colour(0xffdf513e);

const juce::Colour LevelMeter::peakColour =
    juce::Colour(0xffffffff);

const juce::Colour LevelMeter::textColour =
    juce::Colour(0xfff2f2f4);

const juce::Colour LevelMeter::mutedColour =
    juce::Colour(0xff858894);

//==============================================================================
// CONSTRUCTOR
//==============================================================================

LevelMeter::LevelMeter()
{
    setOpaque(false);

    // ----------------------------------------------------------
    // IMPORTANT
    // ----------------------------------------------------------
    //
    // We deliberately do NOT start a Timer here.
    //
    // PluginEditor already has a Timer and will later update
    // this component from its timerCallback().
    //
    // This keeps the meter lightweight and avoids unnecessary
    // timers running independently.
    //
}

//==============================================================================
// LABEL
//==============================================================================

void LevelMeter::setLabel(const juce::String& newLabel)
{
    label = newLabel.toUpperCase();

    repaint();
}

//==============================================================================
// SET LEVEL - LINEAR
//==============================================================================

void LevelMeter::setLevel(float newLevel)
{
    // ----------------------------------------------------------
    // Protect against invalid audio values.
    // ----------------------------------------------------------

    if (! std::isfinite(newLevel))
        newLevel = 0.0f;

    // ----------------------------------------------------------
    // Audio magnitude should never be negative.
    // ----------------------------------------------------------

    newLevel = juce::jmax(0.0f, newLevel);

    // ----------------------------------------------------------
    // Convert linear gain to decibels.
    //
    // -60 dB is used as our visual noise floor.
    // ----------------------------------------------------------

    const float decibels =
        juce::Decibels::gainToDecibels(
            newLevel,
            minimumDecibels
        );

    setLevelDecibels(decibels);
}

//==============================================================================
// SET LEVEL - DECIBELS
//==============================================================================

void LevelMeter::setLevelDecibels(float newLevelDb)
{
    // ----------------------------------------------------------
    // Protect against NaN / infinity.
    // ----------------------------------------------------------

    if (! std::isfinite(newLevelDb))
        newLevelDb = minimumDecibels;

    // ----------------------------------------------------------
    // Keep the value inside our visual range.
    // ----------------------------------------------------------

    newLevelDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            newLevelDb
        );

    targetLevelDb = newLevelDb;

    // ----------------------------------------------------------
    // Peak detection.
    //
    // We only move the peak upward immediately.
    // The peak will later be released by the editor timer.
    // ----------------------------------------------------------

    if (newLevelDb > peakLevelDb)
    {
        peakLevelDb = newLevelDb;
        peakHoldCounter = peakHoldFrames;
    }
}

//==============================================================================
// PEAK
//==============================================================================

void LevelMeter::setPeakDecibels(float newPeakDb)
{
    if (! std::isfinite(newPeakDb))
        newPeakDb = minimumDecibels;

    newPeakDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            newPeakDb
        );

    peakLevelDb = newPeakDb;
    peakHoldCounter = peakHoldFrames;

    repaint();
}

//==============================================================================

void LevelMeter::resetPeak()
{
    peakLevelDb = minimumDecibels;
    peakHoldCounter = 0;

    repaint();
}

//==============================================================================
// RANGE
//==============================================================================

void LevelMeter::setMinimumDecibels(float newMinimumDb)
{
    if (! std::isfinite(newMinimumDb))
        return;

    minimumDecibels = newMinimumDb;

    targetLevelDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            targetLevelDb
        );

    displayedLevelDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            displayedLevelDb
        );

    peakLevelDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            peakLevelDb
        );

    repaint();
}

//==============================================================================

void LevelMeter::setMaximumDecibels(float newMaximumDb)
{
    if (! std::isfinite(newMaximumDb))
        return;

    maximumDecibels = newMaximumDb;

    targetLevelDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            targetLevelDb
        );

    displayedLevelDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            displayedLevelDb
        );

    peakLevelDb =
        juce::jlimit(
            minimumDecibels,
            maximumDecibels,
            peakLevelDb
        );

    repaint();
}

//==============================================================================
// NORMALISE LEVEL
//==============================================================================

float LevelMeter::normaliseLevel(float decibels) const
{
    if (maximumDecibels <= minimumDecibels)
        return 0.0f;

    return juce::jlimit(
        0.0f,
        1.0f,
        (decibels - minimumDecibels)
            / (maximumDecibels - minimumDecibels)
    );
}

//==============================================================================
// ACTIVE SEGMENTS
//==============================================================================

int LevelMeter::getActiveSegmentCount() const
{
    const float normalised =
        normaliseLevel(displayedLevelDb);

    return juce::jlimit(
        0,
        numberOfSegments,
        static_cast<int>(
            std::round(
                normalised *
                static_cast<float>(numberOfSegments)
            )
        )
    );
}

//==============================================================================
// METER BOUNDS
//==============================================================================

juce::Rectangle<float> LevelMeter::getMeterBounds() const
{
    // ----------------------------------------------------------
    // Leave room for:
    //
    // 1. Label
    // 2. Meter
    // 3. dB value
    // 4. Scale markings
    // ----------------------------------------------------------

    auto bounds =
        getLocalBounds()
            .toFloat()
            .reduced(6.0f);

    const float topSpace = 25.0f;
    const float bottomSpace = 42.0f;

    bounds.removeFromTop(topSpace);
    bounds.removeFromBottom(bottomSpace);

    return bounds;
}

//==============================================================================
// SMOOTHING
//==============================================================================

void LevelMeter::updateSmoothing()
{
    // ----------------------------------------------------------
    // Move faster when the signal rises.
    // This makes the meter respond immediately to vocals.
    // ----------------------------------------------------------

    if (targetLevelDb > displayedLevelDb)
    {
        displayedLevelDb +=
            (targetLevelDb - displayedLevelDb)
            * smoothingUp;
    }
    else
    {
        // ------------------------------------------------------
        // Slower release when the signal falls.
        //
        // This creates the professional "falling LED meter"
        // behaviour rather than instantly disappearing.
        // ------------------------------------------------------

        displayedLevelDb +=
            (targetLevelDb - displayedLevelDb)
            * smoothingDown;
    }

    // ----------------------------------------------------------
    // Avoid endless floating-point movement.
    // ----------------------------------------------------------

    if (std::abs(displayedLevelDb - targetLevelDb) < 0.01f)
        displayedLevelDb = targetLevelDb;

    // ----------------------------------------------------------
    // Peak hold release.
    // ----------------------------------------------------------

    if (peakHoldCounter > 0)
    {
        --peakHoldCounter;
    }
    else
    {
        // ------------------------------------------------------
        // Once the hold expires, slowly bring the peak marker
        // down toward the current level.
        // ------------------------------------------------------

        if (peakLevelDb > displayedLevelDb)
        {
            peakLevelDb +=
                (displayedLevelDb - peakLevelDb)
                * 0.08f;
        }
        else
        {
            peakLevelDb = displayedLevelDb;
        }
    }
}

//==============================================================================
// PAINT
//==============================================================================

void LevelMeter::paint(juce::Graphics& g)
{
    // ----------------------------------------------------------
    // Update visual smoothing every time the component paints.
    //
    // PluginEditor will repaint this component periodically.
    // ----------------------------------------------------------

    updateSmoothing();

    auto bounds =
        getLocalBounds()
            .toFloat();

    // ----------------------------------------------------------
    // Component background.
    // ----------------------------------------------------------

    g.setColour(backgroundColour);

    g.fillRoundedRectangle(
        bounds.reduced(1.0f),
        7.0f
    );

    // ----------------------------------------------------------
    // Subtle border.
    // ----------------------------------------------------------

    g.setColour(
        juce::Colour(0xff30333b)
    );

    g.drawRoundedRectangle(
        bounds.reduced(1.0f),
        7.0f,
        1.0f
    );

    // ----------------------------------------------------------
    // LABEL
    // ----------------------------------------------------------

    g.setColour(textColour);

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(11.0f)
                .withStyle("bold")
        )
    );

    g.drawFittedText(
        label,
        juce::Rectangle<int>(
            2,
            5,
            getWidth() - 4,
            16
        ),
        juce::Justification::centred,
        1
    );

    // ----------------------------------------------------------
    // METER
    // ----------------------------------------------------------

    const auto meterBounds =
        getMeterBounds();

    drawMeterBackground(
        g,
        meterBounds
    );

    drawSegments(
        g,
        meterBounds
    );

    drawPeakIndicator(
        g,
        meterBounds
    );

    // ----------------------------------------------------------
    // dB VALUE
    // ----------------------------------------------------------

    drawDbValue(g);

    // ----------------------------------------------------------
    // SCALE
    // ----------------------------------------------------------

    drawScale(
        g,
        meterBounds
    );
}

//==============================================================================
// METER BACKGROUND
//==============================================================================

void LevelMeter::drawMeterBackground(
    juce::Graphics& g,
    juce::Rectangle<float> bounds)
{
    g.setColour(meterBackgroundColour);

    g.fillRoundedRectangle(
        bounds,
        4.0f
    );

    // ----------------------------------------------------------
    // Inner border.
    // ----------------------------------------------------------

    g.setColour(
        juce::Colour(0xff242730)
    );

    g.drawRoundedRectangle(
        bounds,
        4.0f,
        1.0f
    );
}

//==============================================================================
// SEGMENTS
//==============================================================================

void LevelMeter::drawSegments(
    juce::Graphics& g,
    juce::Rectangle<float> bounds)
{
    const int activeSegments =
        getActiveSegmentCount();

    // ----------------------------------------------------------
    // Calculate segment height.
    // ----------------------------------------------------------

    const float totalGap =
        segmentGap *
        static_cast<float>(numberOfSegments - 1);

    const float segmentHeight =
        (bounds.getHeight() - totalGap)
        / static_cast<float>(numberOfSegments);

    // ----------------------------------------------------------
    // Draw from bottom to top.
    // ----------------------------------------------------------

    for (int i = 0; i < numberOfSegments; ++i)
    {
        const int segmentIndex =
            numberOfSegments - 1 - i;

        const float y =
            bounds.getY()
            + static_cast<float>(i)
                * (segmentHeight + segmentGap);

        juce::Rectangle<float> segment(
            bounds.getX() + 3.0f,
            y,
            bounds.getWidth() - 6.0f,
            segmentHeight
        );

        // ------------------------------------------------------
        // Is this segment active?
        // ------------------------------------------------------

        const bool active =
            segmentIndex < activeSegments;

        if (!active)
        {
            // --------------------------------------------------
            // Inactive segment.
            // --------------------------------------------------

            g.setColour(segmentOffColour);

            g.fillRoundedRectangle(
                segment,
                1.5f
            );

            continue;
        }

        // ------------------------------------------------------
        // Determine level region.
        //
        // Bottom:
        //     Green
        //
        // Middle:
        //     Yellow
        //
        // Top:
        //     Red
        // ------------------------------------------------------

        const float position =
            static_cast<float>(segmentIndex)
            / static_cast<float>(numberOfSegments - 1);

        if (position < 0.72f)
        {
            g.setColour(segmentLowColour);
        }
        else if (position < 0.88f)
        {
            g.setColour(segmentMidColour);
        }
        else
        {
            g.setColour(segmentHighColour);
        }

        // ------------------------------------------------------
        // Main LED segment.
        // ------------------------------------------------------

        g.fillRoundedRectangle(
            segment,
            1.5f
        );

        // ------------------------------------------------------
        // Small highlight on the top edge.
        // Gives the segment a more physical LED appearance.
        // ------------------------------------------------------

        g.setColour(
            juce::Colours::white.withAlpha(0.12f)
        );

        g.fillRoundedRectangle(
            juce::Rectangle<float>(
                segment.getX() + 1.0f,
                segment.getY() + 1.0f,
                segment.getWidth() - 2.0f,
                1.0f
            ),
            0.5f
        );
    }
}

//==============================================================================
// PEAK INDICATOR
//==============================================================================

void LevelMeter::drawPeakIndicator(
    juce::Graphics& g,
    juce::Rectangle<float> bounds)
{
    const float normalisedPeak =
        normaliseLevel(peakLevelDb);

    const float peakPosition =
        bounds.getBottom()
        - normalisedPeak
            * bounds.getHeight();

    // ----------------------------------------------------------
    // Only draw the peak indicator if it is inside the meter.
    // ----------------------------------------------------------

    if (peakPosition < bounds.getY()
        || peakPosition > bounds.getBottom())
    {
        return;
    }

    // ----------------------------------------------------------
    // Small white horizontal peak marker.
    // ----------------------------------------------------------

    g.setColour(peakColour);

    g.fillRoundedRectangle(
        juce::Rectangle<float>(
            bounds.getX() + 1.0f,
            peakPosition - 1.0f,
            bounds.getWidth() - 2.0f,
            2.0f
        ),
        1.0f
    );
}

//==============================================================================
// dB VALUE
//==============================================================================

void LevelMeter::drawDbValue(
    juce::Graphics& g)
{
    float displayDb =
        displayedLevelDb;

    // ----------------------------------------------------------
    // Format silence cleanly.
    // ----------------------------------------------------------

    juce::String dbText;

    if (displayDb <= minimumDecibels + 0.5f)
    {
        dbText = "-inf";
    }
    else
    {
        dbText =
            juce::String(
                displayDb,
                1
            )
            + " dB";
    }

    auto valueBounds =
        getLocalBounds()
            .removeFromBottom(27)
            .reduced(2, 0);

    g.setColour(textColour);

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(11.0f)
                .withStyle("bold")
        )
    );

    g.drawFittedText(
        dbText,
        valueBounds,
        juce::Justification::centred,
        1
    );
}

//==============================================================================
// SCALE
//==============================================================================

void LevelMeter::drawScale(
    juce::Graphics& g,
    juce::Rectangle<float> bounds)
{
    // ----------------------------------------------------------
    // Keep scale very subtle.
    //
    // The actual meter remains the main visual element.
    // ----------------------------------------------------------

    g.setFont(
        juce::Font(
            juce::FontOptions()
                .withHeight(8.0f)
        )
    );

    g.setColour(
        mutedColour.withAlpha(0.75f)
    );

    // ----------------------------------------------------------
    // Important reference points.
    // ----------------------------------------------------------

    const float referenceLevels[] =
    {
        -48.0f,
        -24.0f,
        -12.0f,
        -6.0f,
        0.0f
    };

    for (float db : referenceLevels)
    {
        if (db < minimumDecibels
            || db > maximumDecibels)
        {
            continue;
        }

        const float normalised =
            normaliseLevel(db);

        const float y =
            bounds.getBottom()
            - normalised
                * bounds.getHeight();

        // ------------------------------------------------------
        // Small tick.
        // ------------------------------------------------------

        g.fillRect(
            bounds.getRight() - 5.0f,
            y - 0.5f,
            3.0f,
            1.0f
        );
    }
}

//==============================================================================
// RESIZED
//==============================================================================

void LevelMeter::resized()
{
    // ----------------------------------------------------------
    // No child components are used by this meter.
    //
    // Everything is custom-painted, so there is currently
    // nothing to position here.
    // ----------------------------------------------------------
}