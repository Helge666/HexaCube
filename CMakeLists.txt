cmake_minimum_required(VERSION 3.22)
project(HexaCube VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Fetch JUCE ───────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.11
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ─── Embed all 384 samples ────────────────────────────────────────────────────
file(GLOB SAMPLE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Samples/*.wav")
juce_add_binary_data(HexaCubeSamples SOURCES ${SAMPLE_FILES})
set_target_properties(HexaCubeSamples PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# ─── Shared compile definitions ───────────────────────────────────────────────
set(HEXACUBE_COMPILE_DEFS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ─── Shared engine sources (compiled into both targets) ──────────────────────
set(HEXACUBE_ENGINE_SOURCES
    Source/Shared/SampleBank.cpp
    Source/Shared/Voice.cpp
    Source/Shared/VoicePool.cpp
)

# ─── Release plugin ───────────────────────────────────────────────────────────
juce_add_plugin(HexaCube
    COMPANY_NAME                "voXager"
    PLUGIN_MANUFACTURER_CODE    vXgr
    PLUGIN_CODE                 HxCb
    FORMATS                     VST3
    PRODUCT_NAME                "HexaCube"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

juce_generate_juce_header(HexaCube)

target_sources(HexaCube PRIVATE
    ${HEXACUBE_ENGINE_SOURCES}
    Source/Plugin/PluginProcessor.cpp
    Source/Plugin/PluginEditor.cpp
)

target_include_directories(HexaCube PRIVATE Source)
target_compile_definitions(HexaCube PUBLIC ${HEXACUBE_COMPILE_DEFS})

target_link_libraries(HexaCube
    PRIVATE
        HexaCubeSamples
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ─── Config build (developer-only, never ships) ───────────────────────────────
juce_add_plugin(HexaCube_Config
    COMPANY_NAME                "voXager"
    PLUGIN_MANUFACTURER_CODE    vXgr
    PLUGIN_CODE                 HxCf
    FORMATS                     Standalone
    PRODUCT_NAME                "HexaCube Config"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

juce_generate_juce_header(HexaCube_Config)

target_sources(HexaCube_Config PRIVATE
    ${HEXACUBE_ENGINE_SOURCES}
    Source/Config/ConfigProcessor.cpp
    Source/Config/ConfigEditor.cpp
    Source/Config/InstrumentPanel.cpp
)

target_include_directories(HexaCube_Config PRIVATE Source)
target_compile_definitions(HexaCube_Config PUBLIC ${HEXACUBE_COMPILE_DEFS})

target_link_libraries(HexaCube_Config
    PRIVATE
        HexaCubeSamples
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ─── MinGW: statically link runtime so no MinGW DLLs need to ship ────────────
if(MINGW)
    target_link_options(HexaCube      PRIVATE -static-libstdc++ -static-libgcc -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic)
    target_link_options(HexaCube_Config PRIVATE -static-libstdc++ -static-libgcc -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic)
endif()
