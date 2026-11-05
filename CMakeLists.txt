cmake_minimum_required(VERSION 3.22)

# macOS universal (arm64 + x86_64) when configuring on Apple
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS archs" FORCE)
endif()
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "min macOS" FORCE)
endif()

project(Lunchbox303 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        8.0.4
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(juce)

set(LB303_FORMATS VST3 Standalone)
if(APPLE)
  list(APPEND LB303_FORMATS AU)
endif()

juce_add_plugin(Lunchbox303
  COMPANY_NAME "Independent"
  BUNDLE_ID com.independent.lunchbox303
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  PLUGIN_MANUFACTURER_CODE Indp
  PLUGIN_CODE Lb03
  FORMATS ${LB303_FORMATS}
  PRODUCT_NAME "Lunchbox303"
  AU_MAIN_TYPE kAudioUnitType_MusicDevice
)

target_sources(Lunchbox303 PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  Source/VoiceEngine.cpp
  Source/Sequencer.cpp
  Source/Presets.cpp
)

target_compile_definitions(Lunchbox303 PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Lunchbox303 PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  PUBLIC
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)

# Headless offline render / smoke test (no GUI host required)
add_executable(lunchbox303_render Source/OfflineRender.cpp Source/VoiceEngine.cpp Source/Sequencer.cpp Source/Presets.cpp)
target_include_directories(lunchbox303_render PRIVATE Source)
target_compile_features(lunchbox303_render PRIVATE cxx_std_17)
