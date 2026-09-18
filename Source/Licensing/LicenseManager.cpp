#include "LicenseManager.h"
#include <iostream>

// ============================================================
// CONSTRUCTOR
// ============================================================

LicenseManager::LicenseManager()
{
    juce::PropertiesFile::Options options;

    // --------------------------------------------------------
    // Application identity
    // --------------------------------------------------------

    options.applicationName =
        "OfforVocalPro";

    options.filenameSuffix =
        ".settings";

    options.osxLibrarySubFolder =
        "Application Support";

    options.folderName =
        "OFFOR";

    options.storageFormat =
        juce::PropertiesFile::storeAsXML;


    // --------------------------------------------------------
    // Configure application properties
    // --------------------------------------------------------

    properties.setStorageParameters(options);


    // --------------------------------------------------------
    // Make sure this installation has a permanent ID.
    // --------------------------------------------------------

    auto* settings =
        getProperties();

    if (settings != nullptr)
    {
        auto installationId =
            settings->getValue(
                "installationId",
                {}
            );


        // ----------------------------------------------------
        // First installation
        // ----------------------------------------------------

        if (installationId.isEmpty())
        {
            installationId =
                createInstallationId();


            settings->setValue(
                "installationId",
                installationId
            );


            settings->saveIfNeeded();
        }
    }


    // --------------------------------------------------------
    // Load local usage count.
    //
    // This follows the same structure as OfforSampler and
    // OfforStemSplitter.
    //
    // The server remains authoritative when checkUsage()
    // is called.
    // --------------------------------------------------------

    if (settings != nullptr)
    {
        const int localUsage =
            settings->getIntValue(
                "usageCount",
                0
            );


        lastServerFreeUses =
            juce::jmax(
                0,
                localUsage
            );


        // ----------------------------------------------------
        // Vocal Pro has a 10-use trial.
        // ----------------------------------------------------

        lastServerFreeUsesLimit =
            FREE_USES;
    }
}


// ============================================================
// GET PROPERTIES
// ============================================================

juce::PropertiesFile*
LicenseManager::getProperties()
{
    return properties.getUserSettings();
}


// ============================================================
// CREATE INSTALLATION ID
// ============================================================

juce::String
LicenseManager::createInstallationId()
{
    return juce::Uuid()
        .toString()
        .removeCharacters("-");
}


// ============================================================
// INSTALLATION ID
// ============================================================

juce::String
LicenseManager::getInstallationId() const
{
    auto* settings =
        const_cast<juce::ApplicationProperties&>(
            properties
        ).getUserSettings();


    if (settings == nullptr)
        return {};


    return settings->getValue(
        "installationId",
        {}
    );
}


// ============================================================
// ACTIVATED
// ============================================================

bool
LicenseManager::isActivated() const
{
    auto* settings =
        const_cast<juce::ApplicationProperties&>(
            properties
        ).getUserSettings();


    if (settings == nullptr)
        return false;


    return settings->getBoolValue(
        "activated",
        false
    );
}


// ============================================================
// STORED LICENSE
// ============================================================

juce::String
LicenseManager::getStoredLicense() const
{
    auto* settings =
        const_cast<juce::ApplicationProperties&>(
            properties
        ).getUserSettings();


    if (settings == nullptr)
        return {};


    return settings->getValue(
        "licenseKey",
        {}
    );
}


// ============================================================
// USAGE COUNT
// ============================================================

int
LicenseManager::getUsageCount() const
{
    auto* settings =
        const_cast<juce::ApplicationProperties&>(
            properties
        ).getUserSettings();


    if (settings == nullptr)
        return 0;


    return settings->getIntValue(
        "usageCount",
        0
    );
}


// ============================================================
// FREE USES REMAINING
// ============================================================
//
// Offor Vocal Pro:
//
//     FREE_USES = 10
//
// The local usage count is used for the UI/cache.
//
// The server is still checked before allowing a free use.
//

int
LicenseManager::getFreeUsesRemaining() const
{
    // --------------------------------------------------------
    // Activated license = unlimited use.
    // --------------------------------------------------------

    if (isActivated())
        return FREE_USES;


    return juce::jmax(
        0,
        FREE_USES - getUsageCount()
    );
}


// ============================================================
// HAS FREE USES REMAINING
// ============================================================

bool
LicenseManager::hasFreeUsesRemaining() const
{
    // --------------------------------------------------------
    // Activated license = unlimited use.
    // --------------------------------------------------------

    if (isActivated())
        return true;


    return getUsageCount() < FREE_USES;
}


// ============================================================
// SERVER FREE USES
// ============================================================
//
// Runtime information only.
//
// IMPORTANT:
//
// These values are NOT stored in the XML file.
//

int
LicenseManager::getServerFreeUses() const
{
    return lastServerFreeUses;
}


// ============================================================
// SERVER FREE USE LIMIT
// ============================================================

int
LicenseManager::getServerFreeUsesLimit() const
{
    return lastServerFreeUsesLimit;
}


// ============================================================
// LAST SERVER RESULT
// ============================================================

bool
LicenseManager::wasLastServerCheckAllowed() const
{
    return lastServerAllowed;
}


// ============================================================
// INCREMENT LOCAL USAGE
// ============================================================
//
// This is kept for compatibility with the same structure used
// by OfforSampler / OfforStemSplitter.
//
// IMPORTANT:
//
// If /use already increments the server count, the caller
// should NOT call this after checkUsage() unless we explicitly
// decide to use it for a local-only operation.
//
// Our server response is synchronized back into usageCount.
//

void
LicenseManager::incrementUsage()
{
    if (isActivated())
        return;


    auto* settings =
        getProperties();


    if (settings == nullptr)
        return;


    const int currentCount =
        getUsageCount();


    settings->setValue(
        "usageCount",
        currentCount + 1
    );


    settings->saveIfNeeded();
}


// ============================================================
// REGISTER INSTALLATION
// ============================================================

bool
LicenseManager::registerInstallation()
{
    // --------------------------------------------------------
    // Get permanent installation ID.
    // --------------------------------------------------------

    const auto installationId =
        getInstallationId();


    if (installationId.isEmpty())
        return false;


    // --------------------------------------------------------
    // Build JSON request.
    // --------------------------------------------------------

    auto requestObject =
        std::make_unique<juce::DynamicObject>();


    requestObject->setProperty(
        "installationId",
        installationId
    );


    requestObject->setProperty(
        "product",
        PRODUCT_NAME
    );


    requestObject->setProperty(
        "version",
        PRODUCT_VERSION
    );


    juce::var requestVar(
        requestObject.release()
    );


    const auto requestJson =
        juce::JSON::toString(
            requestVar
        );


    // --------------------------------------------------------
    // Create POST URL.
    // --------------------------------------------------------

    juce::URL url(
        juce::String(
            LICENSE_SERVER
        )
        + "/api/v1/license/register"
    );


    url =
        url.withPOSTData(
            requestJson
        );


    int statusCode = 0;


    auto options =
        juce::URL::InputStreamOptions(
            juce::URL::ParameterHandling::inPostData
        )
        .withHttpRequestCmd(
            "POST"
        )
        .withExtraHeaders(
            "Content-Type: application/json\r\n"
            "Accept: application/json"
        )
        .withConnectionTimeoutMs(
            10000
        )
        .withNumRedirectsToFollow(
            5
        )
        .withStatusCode(
            &statusCode
        );


    // --------------------------------------------------------
    // Connect to server.
    // --------------------------------------------------------

    auto stream =
        url.createInputStream(
            options
        );


    if (stream == nullptr)
    {
        std::cout
            <<"License registration failed: "
            <<"no connection."
        << std::endl; 

        return false;
    }


    // --------------------------------------------------------
    // Read response.
    // --------------------------------------------------------

    const auto response =
        stream->readEntireStreamAsString();


    std::cout
        << "License register response: "
        << response
        << std::endl;


    // --------------------------------------------------------
    // HTTP status.
    // --------------------------------------------------------

    if (
        statusCode < 200
        ||
        statusCode >= 300
    )
    {
        std::cout
            << "License registration HTTP status: "
            << statusCode
            << std::endl;

        return false;
    }


    // --------------------------------------------------------
    // Parse JSON.
    // --------------------------------------------------------

    const auto json =
        juce::JSON::parse(
            response
        );


    if (!json.isObject())
        return false;


    const auto* object =
        json.getDynamicObject();


    if (object == nullptr)
        return false;


    // --------------------------------------------------------
    // Check success.
    // --------------------------------------------------------

    const bool success =
        static_cast<bool>(
            object->getProperty(
                "success"
            )
        );


    if (!success)
        return false;


    // --------------------------------------------------------
    // Read server installation information.
    //
    // These values stay in memory.
    // They are NOT saved as separate XML properties.
    // --------------------------------------------------------

    const auto installationVar =
        object->getProperty(
            "installation"
        );


    if (installationVar.isObject())
    {
        const auto* installationObject =
            installationVar.getDynamicObject();


        if (installationObject != nullptr)
        {
            lastServerFreeUses =
                juce::jmax(
                    0,
                    static_cast<int>(
                        installationObject->getProperty(
                            "freeUses"
                        )
                    )
                );


            lastServerFreeUsesLimit =
                juce::jmax(
                    1,
                    static_cast<int>(
                        installationObject->getProperty(
                            "freeUsesLimit"
                        )
                    )
                );


            const bool serverActivated =
                static_cast<bool>(
                    installationObject->getProperty(
                        "activated"
                    )
                );


            // ------------------------------------------------
            // Synchronize only the normal local properties.
            //
            // This keeps Vocal Pro consistent with the other
            // OFFOR products.
            // ------------------------------------------------

            auto* settings =
                getProperties();


            if (settings != nullptr)
            {
                settings->setValue(
                    "usageCount",
                    lastServerFreeUses
                );


                if (serverActivated)
                {
                    settings->setValue(
                        "activated",
                        true
                    );
                }


                settings->saveIfNeeded();
            }
        }
    }


    return true;
}


// ============================================================
// CHECK USAGE WITH SERVER
// ============================================================

bool
LicenseManager::checkUsage()
{
    // --------------------------------------------------------
    // Activated license does not need free-use checking.
    // --------------------------------------------------------

    if (isActivated())
    {
        lastServerAllowed = true;

        return true;
    }


    // --------------------------------------------------------
    // Get installation ID.
    // --------------------------------------------------------

    const auto installationId =
        getInstallationId();


    if (installationId.isEmpty())
    {
        lastServerAllowed = false;

        return false;
    }


    // --------------------------------------------------------
    // Build JSON.
    // --------------------------------------------------------

    auto requestObject =
        std::make_unique<juce::DynamicObject>();


    requestObject->setProperty(
        "installationId",
        installationId
    );


    requestObject->setProperty(
        "product",
        PRODUCT_NAME
    );


    requestObject->setProperty(
        "version",
        PRODUCT_VERSION
    );


    juce::var requestVar(
        requestObject.release()
    );


    const auto requestJson =
        juce::JSON::toString(
            requestVar
        );


    // --------------------------------------------------------
    // Create POST URL.
    // --------------------------------------------------------

    juce::URL url(
        juce::String(
            LICENSE_SERVER
        )
        + "/api/v1/license/use"
    );


    url =
        url.withPOSTData(
            requestJson
        );


    int statusCode = 0;


    auto options =
        juce::URL::InputStreamOptions(
            juce::URL::ParameterHandling::inPostData
        )
        .withHttpRequestCmd(
            "POST"
        )
        .withExtraHeaders(
            "Content-Type: application/json\r\n"
            "Accept: application/json"
        )
        .withConnectionTimeoutMs(
            10000
        )
        .withNumRedirectsToFollow(
            5
        )
        .withStatusCode(
            &statusCode
        );


    // --------------------------------------------------------
    // Connect.
    // --------------------------------------------------------

    auto stream =
        url.createInputStream(
            options
        );


    if (stream == nullptr)
    {
        std::cout
            << "License usage check failed: "
            << "no connection."
            << std::endl;

        lastServerAllowed = false;

        return false;
    }


    // --------------------------------------------------------
    // Read response.
    // --------------------------------------------------------

    const auto response =
        stream->readEntireStreamAsString();


    std::cout
        << "License usage response: "
        << response
        << std::endl;


    // --------------------------------------------------------
    // HTTP status.
    // --------------------------------------------------------

    if (
        statusCode < 200
        ||
        statusCode >= 300
    )
    {
        std::cout
            << "License usage HTTP status: "
            << statusCode
            << std::endl;

        lastServerAllowed = false;

        return false;
    }


    // --------------------------------------------------------
    // Parse JSON.
    // --------------------------------------------------------

    const auto json =
        juce::JSON::parse(
            response
        );


    if (!json.isObject())
    {
        lastServerAllowed = false;

        return false;
    }


    const auto* object =
        json.getDynamicObject();


    if (object == nullptr)
    {
        lastServerAllowed = false;

        return false;
    }


    // ========================================================
    // READ SERVER RESULT
    // ========================================================

    lastServerAllowed =
        static_cast<bool>(
            object->getProperty(
                "allowed"
            )
        );


    lastServerFreeUses =
        juce::jmax(
            0,
            static_cast<int>(
                object->getProperty(
                    "freeUses"
                )
            )
        );


    lastServerFreeUsesLimit =
        juce::jmax(
            1,
            static_cast<int>(
                object->getProperty(
                    "freeUsesLimit"
                )
            )
        );


    // ========================================================
    // SERVER ACTIVATED
    // ========================================================

    const bool serverActivated =
        static_cast<bool>(
            object->getProperty(
                "activated"
            )
        );


    // ========================================================
    // SYNCHRONIZE NORMAL LOCAL PROPERTIES
    // ========================================================
    //
    // Only usageCount and activated are persisted.
    //
    // We intentionally DO NOT store:
    //
    //     serverFreeUses
    //     serverFreeUsesLimit
    //     lastServerAllowed
    //
    // in the XML file.
    //
    // This keeps Vocal Pro consistent with the other products.
    // ========================================================

    auto* settings =
        getProperties();


    if (settings != nullptr)
    {
        settings->setValue(
            "usageCount",
            lastServerFreeUses
        );


        if (serverActivated)
        {
            settings->setValue(
                "activated",
                true
            );
        }


        settings->saveIfNeeded();
    }


    // ========================================================
    // DEBUG
    // ========================================================

    std::cout
        << "OFFOR Vocal Pro - "
        << "Allowed: "
        << (lastServerAllowed ? "YES" : "NO")
        << " | Uses: "
        << lastServerFreeUses
        << " / "
        << lastServerFreeUsesLimit
        << std::endl;
        


    return lastServerAllowed;
}


// ============================================================
// ACTIVATE LICENSE
// ============================================================

bool
LicenseManager::activate(
    const juce::String& licenseKey
)
{
    // --------------------------------------------------------
    // Clean license key.
    // --------------------------------------------------------

    const auto cleanedKey =
        licenseKey.trim();


    if (cleanedKey.isEmpty())
        return false;


    // --------------------------------------------------------
    // Get installation ID.
    // --------------------------------------------------------

    const auto installationId =
        getInstallationId();


    if (installationId.isEmpty())
        return false;


    // --------------------------------------------------------
    // Build JSON.
    // --------------------------------------------------------

    auto requestObject =
        std::make_unique<juce::DynamicObject>();


    requestObject->setProperty(
        "licenseKey",
        cleanedKey
    );


    requestObject->setProperty(
        "installationId",
        installationId
    );


    requestObject->setProperty(
        "product",
        PRODUCT_NAME
    );


    requestObject->setProperty(
        "version",
        PRODUCT_VERSION
    );


    juce::var requestVar(
        requestObject.release()
    );


    const auto requestJson =
        juce::JSON::toString(
            requestVar
        );


    // --------------------------------------------------------
    // Create POST URL.
    // --------------------------------------------------------

    juce::URL url(
        juce::String(
            LICENSE_SERVER
        )
        + "/api/v1/license/activate"
    );


    url =
        url.withPOSTData(
            requestJson
        );


    int statusCode = 0;


    auto options =
        juce::URL::InputStreamOptions(
            juce::URL::ParameterHandling::inPostData
        )
        .withHttpRequestCmd(
            "POST"
        )
        .withExtraHeaders(
            "Content-Type: application/json\r\n"
            "Accept: application/json"
        )
        .withConnectionTimeoutMs(
            10000
        )
        .withNumRedirectsToFollow(
            5
        )
        .withStatusCode(
            &statusCode
        );


    // --------------------------------------------------------
    // Connect.
    // --------------------------------------------------------

    auto stream =
        url.createInputStream(
            options
        );


    if (stream == nullptr)
    {
        std::cout
            << "License activation failed: "
            << "no connection."
            << std::endl;

        return false;
    }


    // --------------------------------------------------------
    // Read response.
    // --------------------------------------------------------

    const auto response =
        stream->readEntireStreamAsString();


    std::cout
        << "License activation response: "
        << response
        << std::endl;


    // --------------------------------------------------------
    // HTTP status.
    // --------------------------------------------------------

    if (
        statusCode < 200
        ||
        statusCode >= 300
    )
    {
        std::cout
            << "License activation HTTP status: "
            << statusCode
            << std::endl;


        return false;
    }


    // --------------------------------------------------------
    // Parse JSON.
    // --------------------------------------------------------

    const auto json =
        juce::JSON::parse(
            response
        );


    if (!json.isObject())
        return false;


    const auto* object =
        json.getDynamicObject();


    if (object == nullptr)
        return false;


    // --------------------------------------------------------
    // Check success.
    // --------------------------------------------------------

    const bool success =
        static_cast<bool>(
            object->getProperty(
                "success"
            )
        );


    if (!success)
        return false;


    // ========================================================
    // SAVE ACTIVATION LOCALLY
    // ========================================================
    //
    // Same structure as OfforSampler / OfforStemSplitter.
    //
    // ========================================================

    auto* settings =
        getProperties();


    if (settings == nullptr)
        return false;


    settings->setValue(
        "activated",
        true
    );


    settings->setValue(
        "licenseKey",
        cleanedKey
    );


    settings->saveIfNeeded();


    // --------------------------------------------------------
    // Update runtime state.
    // --------------------------------------------------------

    lastServerAllowed = true;


    std::cout
        << "OFFOR Vocal Pro - "
        << "License activated successfully."
    << std::endl;


    return true;
}