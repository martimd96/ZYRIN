cmake_minimum_required(VERSION 3.20)
project(ZYRIN VERSION 1.0.0)

# aponta para a pasta onde guardaste o juce
add_subdirectory("D:/PROGRAMAS/Dev/JUCE" juce)

# define as informacoes essenciais do plugin para a daw ler
juce_add_plugin(ZYRIN
    COMPANY_NAME "Prod. Carcyl"
    BUNDLE_ID "com.ProdCarcyl.ZYRIN"
    PLUGIN_MANUFACTURER_CODE "PrCr"
    PLUGIN_CODE "Zyrn"
    FORMATS VST3 Standalone
    PRODUCT_NAME "ZYRIN"
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

# lista todos os ficheiros de codigo que vao ser compilados
target_sources(ZYRIN PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
)

# liga o projeto as bibliotecas basicas de audio e interface
target_link_libraries(ZYRIN PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    ZyrinAssets
)