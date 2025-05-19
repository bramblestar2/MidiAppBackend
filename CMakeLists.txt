cmake_minimum_required(VERSION 3.20)

project(
    MidiApp
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

find_package(rtmidi CONFIG REQUIRED)
find_package(RtAudio CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

add_subdirectory(external/AudioEngine)
add_subdirectory(external/Midi)

add_executable(MidiApp
    src/main.cpp
)

target_link_libraries(MidiApp PRIVATE RtAudio::rtaudio spdlog::spdlog SndFile::sndfile OpenAL::OpenAL RtMidi::rtmidi
    MidiAudioEngine
    MidiManager
)