#include "UpdateChecker.h"

#include <functional>

// ==========================================================
// UPDATE SERVER
// ==========================================================
//
// Express:
//
// app.use(
//     "/api/v1/Offor-Vpro-pdate",
//     OfforVproUpdate
// );
//
// Route:
//
// GET /offor-vocal-pro/update
//
// Final URL:
//
// https://chezchris.onrender.com/api/v1/Offor-Vpro-pdate/offor-vocal-pro/update
//
// ==========================================================

namespace
{
const juce::String updateURL =
"https://chezchris.onrender.com"
"/api/v1/Offor-Vpro-pdate"
"/offor-vocal-pro/update";
}

// ==========================================================
// WORKER THREAD
// ==========================================================

class UpdateChecker::WorkerThread
: private juce::Thread
{
public:

WorkerThread(
    const juce::String& version,
    UpdateChecker::Callback callback)
    : juce::Thread("OFFOR VPRO Update Checker"),
      currentVersion(version),
      resultCallback(std::move(callback))
{
}

~WorkerThread() override
{
    stopThread(11000);
}

void start()
{
    startThread();
}

private:

juce::String currentVersion;

UpdateChecker::Callback resultCallback;

// ======================================================
// VERSION COMPARISON
// ======================================================

static int compareVersions(
    const juce::String& versionA,
    const juce::String& versionB)
{
    auto a =
        juce::StringArray::fromTokens(
            versionA,
            ".",
            "");

    auto b =
        juce::StringArray::fromTokens(
            versionB,
            ".",
            "");

    const int count =
        juce::jmax(
            a.size(),
            b.size());

    for (int i = 0; i < count; ++i)
    {
        const int numberA =
            i < a.size()
                ? a[i].getIntValue()
                : 0;

        const int numberB =
            i < b.size()
                ? b[i].getIntValue()
                : 0;

        if (numberA < numberB)
            return -1;

        if (numberA > numberB)
            return 1;
    }

    return 0;
}

// ======================================================
// THREAD
// ======================================================

void run() override
{
    UpdateChecker::UpdateInfo info;

    // ==================================================
    // CREATE REQUEST
    // ==================================================

    juce::URL url(updateURL);

    auto options =
        juce::URL::InputStreamOptions(
            juce::URL::ParameterHandling::inAddress)
        .withHttpRequestCmd("GET")
        .withExtraHeaders(
            "Accept: application/json\r\n")
        .withConnectionTimeoutMs(10000)
        .withNumRedirectsToFollow(3);

    // ==================================================
    // CONNECT TO SERVER
    // ==================================================

    std::unique_ptr<juce::InputStream> stream;

    try
    {
        stream =
            url.createInputStream(options);
    }
    catch (...)
    {
        info.errorMessage =
            "Unable to connect to update server.";
    }

    if (stream == nullptr)
    {
        if (info.errorMessage.isEmpty())
        {
            info.errorMessage =
                "Unable to connect to update server.";
        }

        sendResult(info);
        return;
    }

    // ==================================================
    // READ SERVER RESPONSE
    // ==================================================

    const auto response =
        stream->readEntireStreamAsString();

    if (response.isEmpty())
    {
        info.errorMessage =
            "Update server returned an empty response.";

        sendResult(info);
        return;
    }

    // ==================================================
    // PARSE JSON
    // ==================================================

    auto json =
        juce::JSON::parse(response);

    if (!json.isObject())
    {
        info.errorMessage =
            "Invalid update server response.";

        sendResult(info);
        return;
    }

    auto object =
        json.getDynamicObject();

    if (object == nullptr)
    {
        info.errorMessage =
            "Invalid update data.";

        sendResult(info);
        return;
    }

    // ==================================================
    // READ SERVER DATA
    // ==================================================

    info.latestVersion =
        object->getProperty(
            "latestVersion").toString();

    info.minimumVersion =
        object->getProperty(
            "minimumVersion").toString();

    info.downloadUrl =
        object->getProperty(
            "downloadUrl").toString();

    // ==================================================
    // RELEASE NOTES
    // ==================================================

    auto releaseNotes =
        object->getProperty(
            "releaseNotes");

    if (auto* array =
            releaseNotes.getArray())
    {
        for (const auto& note : *array)
        {
            info.releaseNotes.add(
                note.toString());
        }
    }

    // ==================================================
    // VALIDATE RESPONSE
    // ==================================================

    if (info.latestVersion.isEmpty())
    {
        info.errorMessage =
            "Server did not provide a latest version.";

        sendResult(info);
        return;
    }

    // ==================================================
    // COMPARE VERSIONS
    // ==================================================

    const int comparison =
        compareVersions(
            currentVersion,
            info.latestVersion);

    info.updateAvailable =
        comparison < 0;

    // ==================================================
    // CHECK MINIMUM VERSION
    // ==================================================

    if (!info.minimumVersion.isEmpty())
    {
        const int minimumComparison =
            compareVersions(
                currentVersion,
                info.minimumVersion);

        info.minimumVersionRequired =
            minimumComparison < 0;
    }

    info.success = true;

    // ==================================================
    // SEND RESULT
    // ==================================================

    sendResult(info);
}

// ======================================================
// SEND RESULT TO MESSAGE THREAD
// ======================================================

void sendResult(
    const UpdateChecker::UpdateInfo& info)
{
    auto callback =
        resultCallback;

    juce::MessageManager::callAsync(
        [callback, info]()
        {
            if (callback)
                callback(info);
        });
}

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
    WorkerThread)
};

// ==========================================================
// CONSTRUCTOR
// ==========================================================

UpdateChecker::UpdateChecker()
{
}

// ==========================================================
// DESTRUCTOR
// ==========================================================

UpdateChecker::~UpdateChecker()
{
workerThread.reset();
}

// ==========================================================
// CHECK FOR UPDATE
// ==========================================================

void UpdateChecker::checkForUpdate(
const juce::String& currentVersion,
Callback callback)
{
// ======================================================
// PREVENT MULTIPLE SIMULTANEOUS CHECKS
// ======================================================


workerThread.reset();

// ======================================================
// CREATE BACKGROUND WORKER
// ======================================================

workerThread =
    std::make_unique<WorkerThread>(
        currentVersion,
        std::move(callback));

workerThread->start();

}
