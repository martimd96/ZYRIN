cmake_minimum_required(VERSION 3.20)
project(ZYRIN VERSION 1.0.0)

# Define the path to JUCE. Users can override this via command line (e.g., -DJUCE_PATH=/path/to/JUCE)
set(JUCE_PATH "JUCE" CACHE PATH "Path to the JUCE framework")
add_subdirectory(${JUCE_PATH} juce)

# Defines essential plugin information for the DAW to read
juce_add_plugin(ZYRIN
    COMPANY_NAME "Prod. Carcyl"
    BUNDLE_ID "com.ProdCarcyl.ZYRIN"
    PLUGIN_MANUFACTURER_CODE "PrCr"
    PLUGIN_CODE "Zyrn"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "ZYRIN"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    VST3_CATEGORIES "Fx|Pitch Shift|Time"
)

juce_generate_juce_header(ZYRIN)

target_compile_definitions(ZYRIN PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Add the BinaryData target for our custom fonts
juce_add_binary_data(ZyrinAssets
    HEADER_NAME "ZyrinAssets.h"
    NAMESPACE ZyrinAssets
    SOURCES
        Assets/Orbitron.ttf
)

# Lists all source code files to be compiled
target_sources(ZYRIN PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
)

# Links the project to the essential audio and UI libraries
target_link_libraries(ZYRIN PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    ZyrinAssets
)