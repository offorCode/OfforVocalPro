cmake_minimum_required(VERSION 3.22)

project(OfforVocalPro VERSION 1.0.0)

# ==========================================================
# JUCE 8.0.14
# ==========================================================

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE"
    "${CMAKE_CURRENT_BINARY_DIR}/JUCE"
)

# ==========================================================
# OFFOR VOCAL PRO
# ==========================================================

juce_add_plugin(OfforVocalPro
    COMPANY_NAME "ONNTECH"
    PRODUCT_NAME "Offor Vocal Pro"

    PLUGIN_MANUFACTURER_CODE ONNR
    PLUGIN_CODE VOCP

    FORMATS VST3 Standalone

    VERSION "1.0.0"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_add_binary_data(OfforVocalProResources
    SOURCES
        Resources/OfforVocalPro.png
        Resources/radio-tuner.png
        Resources/two-arrows.png
        Resources/harmonious.png
        Resources/fx-sign.png
        Resources/startup.png
        Resources/settings.png
)

# ==========================================================
# JUCE HEADER
# ==========================================================

juce_generate_juce_header(OfforVocalPro)

# ==========================================================
# SOURCE FILES
# ==========================================================

target_sources(OfforVocalPro
    PRIVATE

        

        Source/PluginProcessor.cpp
        Source/PluginProcessor.h

        Source/PluginEditor.cpp
        Source/PluginEditor.h

        Source/UI/LevelMeter.cpp
        Source/UI/LevelMeter.h

        Source/UI/SettingsPanel.cpp
        Source/UI/SettingsPanel.h

        Source/UI/ThemeManager.cpp
        Source/UI/ThemeManager.h

        Source/UI/UpdateChecker.h
        Source/UI/UpdateChecker.cpp

        Source/UI/ThemeColorPanel.cpp
        Source/UI/ThemeColorPanel.h

        Source/UI/FeedbackPanel.h
        Source/UI/FeedbackPanel.cpp

        Source/Audio/PitchDetector.cpp
        Source/Audio/PitchDetector.h
        Source/Audio/PitchCorrector.cpp
        Source/Audio/PitchCorrector.h

        Source/Audio/DoublerProcessor.cpp
        Source/Audio/DoublerProcessor.h

        Source/Audio/HarmonyProcessor.h
        Source/Audio/HarmonyProcessor.cpp

        Source/Audio/CreativeFXProcessor.cpp
        Source/Audio/CreativeFXProcessor.h

        Source/Audio/SpaceProcessor.cpp
        Source/Audio/SpaceProcessor.h

        Source/Licensing/LicenseManager.cpp
        Source/Licensing/LicenseManager.h


)

# ==========================================================
# C++ STANDARD
# ==========================================================

target_compile_features(OfforVocalPro
    PRIVATE
        cxx_std_17
)

# ==========================================================
# JUCE MODULES
# ==========================================================

target_link_libraries(OfforVocalPro
    PRIVATE

        OfforVocalProResources

        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp


    PUBLIC

        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ==========================================================
# BUILD DEFINITIONS
# ==========================================================

target_compile_definitions(OfforVocalPro
    PRIVATE

        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0

        JUCE_APPLICATION_NAME_STRING="Offor Vocal Pro"
        JUCE_APPLICATION_VERSION_STRING="1.0.0"

        JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)