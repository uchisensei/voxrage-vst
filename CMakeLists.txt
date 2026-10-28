cmake_minimum_required(VERSION 3.22)

project(VoxRage VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(VoxRage
    COMPANY_NAME "VoxRage Audio"
    BUNDLE_ID "com.voxragaudio.VoxRage"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VrAu
    PLUGIN_CODE VxRg
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "VoxRage"
)

juce_generate_juce_header(VoxRage)

target_sources(VoxRage PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_features(VoxRage PRIVATE cxx_std_20)

target_compile_definitions(VoxRage PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(VoxRage PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)

set_target_properties(VoxRage_VST3 PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
)

if(TARGET VoxRage_AU)
    set_target_properties(VoxRage_AU PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
    )
endif()
