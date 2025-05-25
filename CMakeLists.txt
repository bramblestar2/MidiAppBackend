cmake_minimum_required(VERSION 3.20)

project(
    MidiApp
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

find_package(rtmidi CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

add_subdirectory(external/AudioEngine)
add_subdirectory(external/Midi)

# add_executable
# add_library
add_library(MidiApp
    src/main.cpp
    
    src/App/App.cpp
)

target_link_libraries(MidiApp PRIVATE spdlog::spdlog SndFile::sndfile OpenAL::OpenAL RtMidi::rtmidi
    MidiAudioEngine
    MidiManager
)