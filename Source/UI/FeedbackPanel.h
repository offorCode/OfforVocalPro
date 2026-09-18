#pragma once

#include <JuceHeader.h>

//==============================================================================
// OFFOR VOCAL PRO - FEEDBACK PANEL
//
// Dedicated customer feedback component.
//
// Responsibilities:
//
//      1. Feedback UI
//      2. Feedback type selection
//      3. Name / email / message input
//      4. Validation
//      5. HTTP POST to OFFOR backend
//      6. Background network processing
//      7. Success / error status
//
// SettingsPanel only displays this component.
//==============================================================================

class FeedbackPanel
    : public juce::Component,
      private juce::Thread
{
public:

    //==========================================================================

    FeedbackPanel();

    ~FeedbackPanel() override;

    //==========================================================================

    void paint(
        juce::Graphics& g) override;

    void resized() override;

    //==========================================================================

    // Re-applies the current OFFOR theme.
    //
    // SettingsPanel calls this whenever ThemeManager changes.
    //
    void updateThemeColours();

private:

    //==========================================================================

    // HTTP ENDPOINT
    //
    // This is the public backend endpoint created for plugin feedback.
    //
    // If your backend domain changes in the future, change this ONE line.
    //
    static constexpr const char* feedbackEndpoint =
        "https://chezchris.onrender.com/api/v1/plugin-feedback";

    //==========================================================================

    // FEEDBACK UI
    //==========================================================================

    juce::Label titleLabel;
    juce::Label descriptionLabel;

    juce::Label typeLabel;
    juce::Label nameLabel;
    juce::Label emailLabel;
    juce::Label messageLabel;

    juce::ComboBox typeComboBox;

    juce::TextEditor nameEditor;
    juce::TextEditor emailEditor;
    juce::TextEditor messageEditor;

    juce::TextButton sendButton;

    juce::Label statusLabel;

    //==========================================================================

    // NETWORK STATE
    //==========================================================================

    bool sendingFeedback = false;

    //==========================================================================

    // SETUP
    //==========================================================================

    void setupLabels();

    void setupControls();

    void setupCallbacks();

    //==========================================================================

    // FEEDBACK
    //==========================================================================

    void submitFeedback();

    void setStatus(
        const juce::String& message,
        bool success);

    void clearForm();

    //==========================================================================

    // BACKGROUND NETWORK THREAD
    //==========================================================================

    void run() override;

    //==========================================================================

    // SYSTEM INFORMATION
    //==========================================================================

    juce::String getSystemInformation() const;

    //==========================================================================

    // JSON
    //==========================================================================

    juce::String createFeedbackJSON() const;

    //==========================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        FeedbackPanel)
};