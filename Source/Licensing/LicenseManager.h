#pragma once

#include <JuceHeader.h>


// ============================================================
// OFFOR VOCAL PRO LICENSE MANAGER
// ============================================================
//
// Handles:
//
// 1. Permanent installation ID
// 2. Server registration
// 3. Free-use tracking
// 4. Server usage validation
// 5. License activation
// 6. Local license persistence
//
// ============================================================

class LicenseManager
{
public:

    // ========================================================
    // PRODUCT INFORMATION
    // ========================================================

    static constexpr const char* PRODUCT_NAME =
        "OfforVocalPro";

    static constexpr const char* PRODUCT_VERSION =
        "1.0.0";


    // ========================================================
    // FREE TRIAL
    // ========================================================
    //
    // Offor Vocal Pro gives users 10 free uses.
    //
    // IMPORTANT:
    //
    // The server also enforces this limit.
    // This value is mainly used by the client for local
    // calculations and UI.
    //
    // ========================================================

    static constexpr int FREE_USES = 10;


    // ========================================================
    // LICENSE SERVER
    // ========================================================

    static constexpr const char* LICENSE_SERVER =
        "https://chezchris.onrender.com";


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    LicenseManager();


    // ========================================================
    // INSTALLATION
    // ========================================================

    juce::String getInstallationId() const;


    // ========================================================
    // LICENSE STATUS
    // ========================================================

    bool isActivated() const;

    juce::String getStoredLicense() const;


    // ========================================================
    // LOCAL USAGE
    // ========================================================

    int getUsageCount() const;

    int getFreeUsesRemaining() const;

    bool hasFreeUsesRemaining() const;


    // ========================================================
    // SERVER USAGE INFORMATION
    // ========================================================
    //
    // Runtime information only.
    //
    // These values are NOT saved as separate XML properties.
    //
    // ========================================================

    int getServerFreeUses() const;

    int getServerFreeUsesLimit() const;

    bool wasLastServerCheckAllowed() const;


    // ========================================================
    // USAGE
    // ========================================================

    void incrementUsage();


    // ========================================================
    // SERVER
    // ========================================================

    bool registerInstallation();

    bool checkUsage();


    // ========================================================
    // LICENSE ACTIVATION
    // ========================================================

    bool activate(
        const juce::String& licenseKey
    );


private:

    // ========================================================
    // JUCE APPLICATION PROPERTIES
    // ========================================================

    juce::ApplicationProperties properties;


    // ========================================================
    // RUNTIME SERVER STATE
    // ========================================================
    //
    // These are kept in memory only.
    //
    // They are NOT written into the user's XML file.
    //
    // ========================================================

    bool lastServerAllowed = false;

    int lastServerFreeUses = 0;

    int lastServerFreeUsesLimit = FREE_USES;


    // ========================================================
    // INSTALLATION ID
    // ========================================================

    juce::String createInstallationId();


    // ========================================================
    // PROPERTY ACCESS
    // ========================================================

    juce::PropertiesFile* getProperties();


    // ========================================================
    // NON-COPYABLE
    // ========================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        LicenseManager
    )
};