cmake_minimum_required(VERSION 3.20)

project(
    MidiApp
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

set(GENERATE_LIB "ON" CACHE STRING "Generate library")

find_package(rtmidi CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(external/AudioEngine)
add_subdirectory(external/Midi)

# add_executable
# add_library
add_executable(MidiApp
    src/main.cpp
    
    src/App/App.cpp
    src/App/IdPool.cpp
    src/App/MidiBindingBuilder.cpp
    src/App/MidiBindingManager.cpp
    src/App/json.cpp
)

target_include_directories(MidiApp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(MidiApp PUBLIC 
    spdlog::spdlog 
    SndFile::sndfile 
    OpenAL::OpenAL 
    RtMidi::rtmidi
    nlohmann_json::nlohmann_json

    improved_rust_audio
    MidiManager
)

add_dependencies(MidiApp
    improved_rust_audio
    MidiManager
)

target_include_directories(improved_rust_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)