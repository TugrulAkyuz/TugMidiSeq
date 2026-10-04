# Engine regression tests and screenshot renderer for TugMidiSeq.
# A plain console app built from the plugin's own Source/ files; it has nothing
# to do with the Projucer build. See README.md.
cmake_minimum_required(VERSION 3.22)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# the plugin's version, so the editor's version stamp matches the release
file(READ ${REPO_ROOT}/TugMidiSeq.jucer JUCER)
string(REGEX MATCH "version=\"([0-9]+\\.[0-9]+\\.[0-9]+)\"[^>]*pluginAUMainType" _ "${JUCER}")
if(NOT CMAKE_MATCH_1)
    set(CMAKE_MATCH_1 0.0.0)
endif()
project(TugMidiSeqEngineTest VERSION ${CMAKE_MATCH_1})

# JUCE sits next to 2RuleProgramming, as in the .jucer (../../JUCE/modules)
set(JUCE_DIR ${REPO_ROOT}/../../JUCE CACHE PATH "JUCE checkout")
add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/juce)

set(SRC ${REPO_ROOT}/Source)

juce_add_console_app(EngineTest PRODUCT_NAME "EngineTest")
juce_generate_juce_header(EngineTest)

target_sources(EngineTest PRIVATE
    main.cpp
    ${SRC}/PluginProcessor.cpp
    ${SRC}/PluginEditor.cpp
    ${SRC}/PresetMenu.cpp
    ${SRC}/Grids.cpp
    ${SRC}/GlobalPanel.cpp
    ${SRC}/Satellite.cpp
    ${SRC}/SynthVoice.cpp
    ${SRC}/NoteMap.cpp)

target_include_directories(EngineTest PRIVATE ${SRC})

target_compile_definitions(EngineTest PRIVATE
    JucePlugin_Name="TugMidiSeq"
    JucePlugin_IsSynth=1
    JucePlugin_WantsMidiInput=1
    JucePlugin_ProducesMidiOutput=1
    JucePlugin_IsMidiEffect=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_MODAL_LOOPS_PERMITTED=1   # lets the screenshots run the GUI timers first
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)

target_compile_options(EngineTest PRIVATE -w)

target_link_libraries(EngineTest PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_recommended_config_flags)
