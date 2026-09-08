cmake_minimum_required(VERSION 3.22)
project(ZenithGranular VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE é buscado automaticamente do GitHub (framework gratuito/open-source).
# Precisa de internet na primeira vez que correres o cmake.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)
FetchContent_MakeAvailable(JUCE)

# IS_SYNTH TRUE + NEEDS_MIDI_INPUT TRUE = plugin de instrumento (Channel Rack no FL Studio),
# conforme decidido: sem modo de efeito de mixer na v1.
juce_add_plugin(ZenithGranular
    COMPANY_NAME "ZenithWeb"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Zwbb
    PLUGIN_CODE Zgr1
    FORMATS VST3 AU
    PRODUCT_NAME "ZenithGranular")

target_sources(ZenithGranular
    PRIVATE
        PluginProcessor.cpp
        PluginEditor.cpp
        SamplerEngine.cpp
        FilterEngine.cpp
        SaturationEngine.cpp
        DelayEngine.cpp
        ReverbEngine.cpp
        PresetManager.cpp
        ZenithLookAndFeel.cpp
        WaveformDisplay.cpp)

target_compile_definitions(ZenithGranular
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        $<$<PLATFORM_ID:Windows>:JUCE_USE_WINDOWS_MEDIA_FORMAT=1>)

target_link_libraries(ZenithGranular
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
