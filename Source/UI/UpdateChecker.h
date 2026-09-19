#pragma once

#include <JuceHeader.h>

// ==========================================================
// OFFOR VOCAL PRO UPDATE CHECKER
// ==========================================================
//
// Checks the ChezChris server for a newer version of
// Offor Vocal Pro.
//
// IMPORTANT:
// This class must NEVER be called from the audio thread.
//
// ==========================================================

class UpdateChecker
{
public:

// ======================================================
// UPDATE INFORMATION
// ======================================================

struct UpdateInfo
{
    bool success = false;

    bool updateAvailable = false;

    bool minimumVersionRequired = false;

    juce::String latestVersion;

    juce::String minimumVersion;

    juce::String downloadUrl;

    juce::StringArray releaseNotes;

    juce::String errorMessage;
};

// ======================================================
// CALLBACK
// ======================================================

using Callback =
    std::function<void(const UpdateInfo&)>;

// ======================================================
// CONSTRUCTOR / DESTRUCTOR
// ======================================================

UpdateChecker();

~UpdateChecker();

// ======================================================
// CHECK FOR UPDATE
// ======================================================

void checkForUpdate(
    const juce::String& currentVersion,
    Callback callback);

private:


class WorkerThread;

std::unique_ptr<WorkerThread> workerThread;

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UpdateChecker)

};
