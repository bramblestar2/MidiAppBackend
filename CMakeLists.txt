cmake_minimum_required(VERSION 3.24)

project(
    MidiApp
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(rtmidi CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
    improved_rust_audio
    GIT_REPOSITORY https://github.com/bramblestar2/ImprovedRustAudio.git
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(improved_rust_audio)

FetchContent_Declare(
    MidiReworkCore
    GIT_REPOSITORY https://github.com/bramblestar2/MidiRework.git
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(MidiReworkCore)


# LIBRARY

add_library(MidiAppCore STATIC
    src/App/App.cpp
    src/App/IdPool.cpp
    src/App/MidiBindingBuilder.cpp
    src/App/MidiBindingManager.cpp
    src/App/json.cpp
)

target_include_directories(MidiAppCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MidiAppCore
    PUBLIC
        spdlog::spdlog
        SndFile::sndfile
        OpenAL::OpenAL
        RtMidi::rtmidi
        nlohmann_json::nlohmann_json
        unofficial::sqlite3::sqlite3

        improved_rust_audio
        MidiReworkCore
)


# EXECUTABLE

add_executable(MidiApp
    src/main.cpp
)

target_link_libraries(MidiApp
    PRIVATE
        MidiAppCore
)
