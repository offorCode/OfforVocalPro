#include "FeedbackPanel.h"

#include "ThemeManager.h"
#include "../Version.h"

//==============================================================================
// CONSTRUCTOR
//==============================================================================

FeedbackPanel::FeedbackPanel()
    : juce::Thread("OFFOR Feedback Thread")
{
    setOpaque(false);

    setupLabels();
    setupControls();
    setupCallbacks();

    updateThemeColours();
}

//==============================================================================
// DESTRUCTOR
//==============================================================================

FeedbackPanel::~FeedbackPanel()
{
    //==========================================================================
    // IMPORTANT
    //
    // Never destroy the panel while the network thread is still using it.
    //
    // The thread is stopped before the component is destroyed.
    //==========================================================================

    stopThread(10000);
}

//==============================================================================
// SETUP LABELS
//==============================================================================

void FeedbackPanel::setupLabels()
{
    //==========================================================================
    // TITLE
    //==========================================================================

    titleLabel.setText(
        "FEEDBACK",
        juce::dontSendNotification);

    titleLabel.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(20.0f)
                .withStyle("Bold")));

    titleLabel.setJustificationType(
        juce::Justification::centredLeft);

    addAndMakeVisible(
        titleLabel);

    //==========================================================================
    // DESCRIPTION
    //==========================================================================

    descriptionLabel.setText(
        "Help us improve OFFOR Vocal Pro. "
        "Send a bug report, feature request or general feedback.",
        juce::dontSendNotification);

    descriptionLabel.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)
                .withStyle("Medium")));

    descriptionLabel.setJustificationType(
        juce::Justification::centredLeft);

    addAndMakeVisible(
        descriptionLabel);

    //==========================================================================
    // FIELD LABELS
    //==========================================================================

    typeLabel.setText(
        "FEEDBACK TYPE",
        juce::dontSendNotification);

    nameLabel.setText(
        "NAME",
        juce::dontSendNotification);

    emailLabel.setText(
        "EMAIL",
        juce::dontSendNotification);

    messageLabel.setText(
        "MESSAGE",
        juce::dontSendNotification);

    juce::Label* labels[] =
    {
        &typeLabel,
        &nameLabel,
        &emailLabel,
        &messageLabel
    };

    for (auto* label : labels)
    {
        label->setFont(
            juce::Font(
                juce::FontOptions{}
                    .withName("Poppins")
                    .withHeight(10.0f)
                    .withStyle("Bold")));

        label->setJustificationType(
            juce::Justification::centredLeft);

        addAndMakeVisible(
            *label);
    }

    //==========================================================================
    // STATUS
    //==========================================================================

    statusLabel.setText(
        "",
        juce::dontSendNotification);

    statusLabel.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(10.0f)
                .withStyle("Medium")));

    statusLabel.setJustificationType(
        juce::Justification::centredLeft);

    addAndMakeVisible(
        statusLabel);
}

//==============================================================================
// SETUP CONTROLS
//==============================================================================

void FeedbackPanel::setupControls()
{
    //==========================================================================
    // FEEDBACK TYPE
    //==========================================================================

    typeComboBox.addItem(
        "Bug",
        1);

    typeComboBox.addItem(
        "Feature Request",
        2);

    typeComboBox.addItem(
        "General Feedback",
        3);

    typeComboBox.setSelectedId(
        3,
        juce::dontSendNotification);

    typeComboBox.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(
        typeComboBox);

    //==========================================================================
    // NAME
    //==========================================================================

    nameEditor.setMultiLine(false);
    nameEditor.setReturnKeyStartsNewLine(false);
    nameEditor.setScrollbarsShown(false);
    nameEditor.setTextToShowWhenEmpty(
        "Your name",
        juce::Colours::grey);

    nameEditor.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)));

    addAndMakeVisible(
        nameEditor);

    //==========================================================================
    // EMAIL
    //==========================================================================

    emailEditor.setMultiLine(false);
    emailEditor.setReturnKeyStartsNewLine(false);
    emailEditor.setScrollbarsShown(false);
    emailEditor.setTextToShowWhenEmpty(
        "Your email address (optional)",
        juce::Colours::grey);

    emailEditor.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)));

    addAndMakeVisible(
        emailEditor);

    //==========================================================================
    // MESSAGE
    //==========================================================================

    messageEditor.setMultiLine(true);
    messageEditor.setReturnKeyStartsNewLine(true);
    messageEditor.setScrollbarsShown(true);
    messageEditor.setTextToShowWhenEmpty(
        "Tell us what happened, what you would like to see, "
        "or what you think about OFFOR Vocal Pro...",
        juce::Colours::grey);

    messageEditor.setFont(
        juce::Font(
            juce::FontOptions{}
                .withName("Poppins")
                .withHeight(11.0f)));

    addAndMakeVisible(
        messageEditor);

    //==========================================================================
    // SEND BUTTON
    //==========================================================================

    sendButton.setButtonText(
        "SEND FEEDBACK");

    sendButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    sendButton.setWantsKeyboardFocus(false);

    addAndMakeVisible(
        sendButton);
}

//==============================================================================
// SETUP CALLBACKS
//==============================================================================

void FeedbackPanel::setupCallbacks()
{
    sendButton.onClick =
        [this]()
        {
            submitFeedback();
        };
}

//==============================================================================
// SUBMIT FEEDBACK
//==============================================================================

void FeedbackPanel::submitFeedback()
{
    //==========================================================================
    // PREVENT MULTIPLE SUBMISSIONS
    //==========================================================================

    if (sendingFeedback)
        return;

    //==========================================================================
    // GET MESSAGE
    //==========================================================================

    const auto message =
        messageEditor.getText().trim();

    if (message.isEmpty())
    {
        setStatus(
            "Please enter a message.",
            false);

        messageEditor.grabKeyboardFocus();

        return;
    }

    //==========================================================================
    // LIMIT MESSAGE SIZE
    //==========================================================================

    if (message.length() > 5000)
    {
        setStatus(
            "Message is too long. Maximum 5000 characters.",
            false);

        return;
    }

    //==========================================================================
    // GET NAME
    //==========================================================================

    const auto name =
        nameEditor.getText().trim();

    //==========================================================================
    // GET EMAIL
    //==========================================================================

    const auto email =
        emailEditor.getText().trim();

    //==========================================================================
    // BASIC EMAIL VALIDATION
    //
    // Email is optional.
    // If supplied, make sure it at least looks like an email address.
    //==========================================================================

    if (!email.isEmpty())
    {
        if (!email.contains("@") ||
            !email.contains("."))
        {
            setStatus(
                "Please enter a valid email address.",
                false);

            emailEditor.grabKeyboardFocus();

            return;
        }
    }

    //==========================================================================
    // PREPARE UI
    //==========================================================================

    sendingFeedback = true;

    sendButton.setEnabled(false);

    sendButton.setButtonText(
        "SENDING...");

    setStatus(
        "Sending feedback...",
        false);

    //==========================================================================
    // START BACKGROUND THREAD
    //
    // IMPORTANT:
    //
    // HTTP requests must NEVER run directly on the audio thread.
    // They also should not block the JUCE message thread.
    //
    //==========================================================================

    if (isThreadRunning())
    {
        sendingFeedback = false;
        sendButton.setEnabled(true);
        sendButton.setButtonText("SEND FEEDBACK");

        setStatus(
            "Another feedback request is already running.",
            false);

        return;
    }

    startThread();
}

//==============================================================================
// CREATE FEEDBACK JSON
//==============================================================================

juce::String FeedbackPanel::createFeedbackJSON() const
{
    //==========================================================================
    // FEEDBACK TYPE
    //==========================================================================

    juce::String feedbackType =
        typeComboBox.getText();

    if (feedbackType.isEmpty())
        feedbackType = "General Feedback";

    //==========================================================================
    // CREATE JSON OBJECT
    //==========================================================================

    auto object =
        std::make_unique<juce::DynamicObject>();

    object->setProperty(
        "name",
        nameEditor.getText().trim().isEmpty()
            ? "Guest"
            : nameEditor.getText().trim());

    object->setProperty(
        "email",
        emailEditor.getText().trim());

    object->setProperty(
        "type",
        feedbackType);

    object->setProperty(
        "message",
        messageEditor.getText().trim());

    //==========================================================================
    // AUTOMATIC PLUGIN INFORMATION
    //==========================================================================

    object->setProperty(
        "plugin",
        "Offor Vocal Pro");

    object->setProperty(
        "version",
        OFFOR_VPRO_VERSION_STRING);

    //==========================================================================
    // AUTOMATIC SYSTEM INFORMATION
    //==========================================================================

    object->setProperty(
        "system",
        getSystemInformation());

    //==========================================================================
    // CONVERT OBJECT TO JSON
    //==========================================================================

    return juce::JSON::toString(
        juce::var(
            object.release()));
}

//==============================================================================
// SYSTEM INFORMATION
//==============================================================================

juce::String FeedbackPanel::getSystemInformation() const
{
    juce::String information;

    //==========================================================================
    // OPERATING SYSTEM
    //==========================================================================

    information +=
        juce::SystemStats::getOperatingSystemName();

    //==========================================================================
    // CPU
    //==========================================================================

    const auto cpu =
        juce::SystemStats::getCpuModel();

    if (cpu.isNotEmpty())
    {
        information +=
            " | CPU: " + cpu;
    }

    //==========================================================================
    // JUCE VERSION
    //==========================================================================

    information +=
        " | JUCE: " + juce::SystemStats::getJUCEVersion();

    return information;
}

//==============================================================================
// BACKGROUND THREAD
//==============================================================================

void FeedbackPanel::run()
{
    //==========================================================================
    // CREATE JSON
    //==========================================================================

    const auto json =
        createFeedbackJSON();

    //==========================================================================
    // CREATE URL
    //==========================================================================

    juce::URL url(
        feedbackEndpoint);

    //==========================================================================
    // ATTACH JSON AS POST DATA
    //==========================================================================

    url =
        url.withPOSTData(
            json);

    //==========================================================================
    // HTTP OPTIONS
    //==========================================================================

    auto options =
        juce::URL::InputStreamOptions(
            juce::URL::ParameterHandling::inPostData)
            .withHttpRequestCmd("POST")
            .withExtraHeaders(
                "Content-Type: application/json\r\n"
                "Accept: application/json\r\n")
            .withConnectionTimeoutMs(
                10000)
            .withNumRedirectsToFollow(
                3);

    //==========================================================================
    // CREATE NETWORK STREAM
    //==========================================================================

    auto stream =
        url.createInputStream(
            options);

    //==========================================================================
    // NETWORK FAILURE
    //==========================================================================

    if (stream == nullptr)
    {
        juce::MessageManager::callAsync(
            [safeThis = juce::Component::SafePointer<FeedbackPanel>(
                 this)]()
            {
                if (safeThis == nullptr)
                    return;

                safeThis->sendingFeedback =
                    false;

                safeThis->sendButton.setEnabled(
                    true);

                safeThis->sendButton.setButtonText(
                    "SEND FEEDBACK");

                safeThis->setStatus(
                    "Could not connect to the OFFOR server.",
                    false);
            });

        return;
    }

    //==========================================================================
    // READ SERVER RESPONSE
    //==========================================================================

    const auto response =
        stream->readEntireStreamAsString();

    //==========================================================================
    // PARSE RESPONSE
    //==========================================================================

    const auto responseJSON =
        juce::JSON::parse(
            response);

    bool success = false;

    juce::String serverMessage;

    if (responseJSON.isObject())
    {
        success =
            static_cast<bool>(
                responseJSON.getProperty(
                    "success",
                    false));

        serverMessage =
            responseJSON.getProperty(
                "message",
                "").toString();
    }

    //==========================================================================
    // SEND RESULT BACK TO JUCE MESSAGE THREAD
    //==========================================================================

    juce::MessageManager::callAsync(
        [safeThis = juce::Component::SafePointer<FeedbackPanel>(
             this),
         success,
         serverMessage]()
        {
            if (safeThis == nullptr)
                return;

            safeThis->sendingFeedback =
                false;

            safeThis->sendButton.setEnabled(
                true);

            safeThis->sendButton.setButtonText(
                "SEND FEEDBACK");

            if (success)
            {
                safeThis->setStatus(
                    "Thank you. Your feedback has been sent.",
                    true);

                safeThis->clearForm();
            }
            else
            {
                const auto message =
                    serverMessage.isNotEmpty()
                        ? serverMessage
                        : "The server rejected the feedback.";

                safeThis->setStatus(
                    message,
                    false);
            }
        });
}

//==============================================================================
// SET STATUS
//==============================================================================

void FeedbackPanel::setStatus(
    const juce::String& message,
    bool success)
{
    statusLabel.setText(
        message,
        juce::dontSendNotification);

    const auto colours =
        ThemeManager::get().getColours();

    statusLabel.setColour(
        juce::Label::textColourId,
        success
            ? colours.success
            : colours.muted);

    statusLabel.repaint();
}

//==============================================================================
// CLEAR FORM
//==============================================================================

void FeedbackPanel::clearForm()
{
    nameEditor.clear();
    emailEditor.clear();
    messageEditor.clear();

    typeComboBox.setSelectedId(
        3,
        juce::dontSendNotification);
}

//==============================================================================
// THEME
//==============================================================================

void FeedbackPanel::updateThemeColours()
{
    const auto colours =
        ThemeManager::get().getColours();

    //==========================================================================
    // LABELS
    //==========================================================================

    titleLabel.setColour(
        juce::Label::textColourId,
        colours.text);

    descriptionLabel.setColour(
        juce::Label::textColourId,
        colours.muted);

    typeLabel.setColour(
        juce::Label::textColourId,
        colours.accent);

    nameLabel.setColour(
        juce::Label::textColourId,
        colours.accent);

    emailLabel.setColour(
        juce::Label::textColourId,
        colours.accent);

    messageLabel.setColour(
        juce::Label::textColourId,
        colours.accent);

    //==========================================================================
    // TEXT EDITORS
    //==========================================================================

    juce::TextEditor* editors[] =
    {
        &nameEditor,
        &emailEditor,
        &messageEditor
    };

    for (auto* editor : editors)
    {
        editor->setColour(
            juce::TextEditor::backgroundColourId,
            colours.panel2);

        editor->setColour(
            juce::TextEditor::textColourId,
            colours.text);

        editor->setColour(
            juce::TextEditor::outlineColourId,
            colours.border);

        editor->setColour(
            juce::TextEditor::focusedOutlineColourId,
            colours.accent);

        editor->setColour(
            juce::TextEditor::highlightColourId,
            colours.accent.withAlpha(0.30f));

        editor->setColour(
            juce::TextEditor::highlightedTextColourId,
            colours.text);

        editor->repaint();
    }

    //==========================================================================
    // COMBO BOX
    //==========================================================================

    typeComboBox.setColour(
        juce::ComboBox::backgroundColourId,
        colours.panel2);

    typeComboBox.setColour(
        juce::ComboBox::textColourId,
        colours.text);

    typeComboBox.setColour(
        juce::ComboBox::outlineColourId,
        colours.border);

    typeComboBox.setColour(
        juce::ComboBox::arrowColourId,
        colours.muted);

    typeComboBox.repaint();

    //==========================================================================
    // SEND BUTTON
    //==========================================================================

    sendButton.setColour(
        juce::TextButton::buttonColourId,
        colours.accent);

    sendButton.setColour(
        juce::TextButton::buttonOnColourId,
        colours.accentDark);

    sendButton.setColour(
        juce::TextButton::textColourOffId,
        colours.text);

    sendButton.setColour(
        juce::TextButton::textColourOnId,
        colours.text);

    sendButton.repaint();

    //==========================================================================
    // STATUS
    //==========================================================================

    statusLabel.repaint();
}

//==============================================================================
// PAINT
//==============================================================================

void FeedbackPanel::paint(
    juce::Graphics& g)
{
    const auto colours =
        ThemeManager::get().getColours();

    //==========================================================================
    // INPUT / CONTROL BACKGROUND
    //==========================================================================

    auto drawEditorBackground =
        [&g, &colours](const juce::Component& component)
        {
            auto bounds =
                component.getBounds()
                    .toFloat();

            g.setColour(
                colours.panel2.withAlpha(0.65f));

            g.fillRoundedRectangle(
                bounds,
                7.0f);

            g.setColour(
                colours.border.withAlpha(0.65f));

            g.drawRoundedRectangle(
                bounds.reduced(0.5f),
                7.0f,
                1.0f);
        };

    // The actual controls paint themselves.
    juce::ignoreUnused(
        drawEditorBackground);
}

//==============================================================================
// RESIZED
//==============================================================================

void FeedbackPanel::resized()
{
    const int width =
        getWidth();

    const int left =
        0;

    const int right =
        width;

    const int controlWidth =
        260;

    const int controlRight =
        right - controlWidth;

    const int fieldHeight =
        34;

    //==========================================================================
    // HEADER
    //==========================================================================

    titleLabel.setBounds(
        left,
        0,
        width,
        30);

    descriptionLabel.setBounds(
        left,
        32,
        width,
        42);

    //==========================================================================
    // FEEDBACK TYPE
    //==========================================================================

    typeLabel.setBounds(
        left,
        82,
        180,
        20);

    typeComboBox.setBounds(
        controlRight,
        78,
        controlWidth,
        fieldHeight);

    //==========================================================================
    // NAME
    //==========================================================================

    nameLabel.setBounds(
        left,
        132,
        180,
        20);

    nameEditor.setBounds(
        controlRight,
        128,
        controlWidth,
        fieldHeight);

    //==========================================================================
    // EMAIL
    //==========================================================================

    emailLabel.setBounds(
        left,
        182,
        180,
        20);

    emailEditor.setBounds(
        controlRight,
        178,
        controlWidth,
        fieldHeight);

    //==========================================================================
    // MESSAGE
    //==========================================================================

    messageLabel.setBounds(
        left,
        232,
        180,
        20);

    messageEditor.setBounds(
        left,
        260,
        width,
        145);

    //==========================================================================
    // SEND BUTTON
    //==========================================================================

    sendButton.setBounds(
        width - 170,
        420,
        170,
        38);

    //==========================================================================
    // STATUS
    //==========================================================================

    statusLabel.setBounds(
        left,
        420,
        width - 185,
        38);
}
