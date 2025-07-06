#pragma once
#include <map>

#include <spdlog/spdlog.h>
// #include <Audio/Audio_Includes.h>
#include <Midi/MidiManager.h>

#include <Audio/Audio.h>

#include <functional>

#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <optional>
#include <memory>

#include <nlohmann/json.hpp>

#include "IdPool.h"
#include "types.h"
#include "MidiBindingBuilder.h"
#include "MidiBindingManager.h"

class App {
public:
    App();

    void setCurrentPage(const int page);
    const int& getCurrentPage() const { return m_currentPage; }

    void onMidiCallback(std::function<void(MidiDevice*, MidiMessage)> callback);
    void onMidiBindingsChanged(std::function<void()> callback);
    void onAudioListChanged(std::function<void()> callback);
    void onPageChanged(std::function<void(int)> callback);
    void onDeviceRefresh(std::function<void(std::vector<MidiDevice*>)> callback);

    //returns binding ID
    int addMidiBinding(MidiBinding binding);
    void removeMidiBinding(const int id);
    
    MidiBindingBuilder midiBind(const std::string &deviceName);

    const std::map<int, std::vector<MidiBinding>>& getMidiBindingPages() const;
    std::vector<MidiBinding*> getMidiBindingsForPage(int page);
    std::vector<MidiBinding*> getMidiBindings();

    void handleMidiMessage(MidiDevice* device, MidiMessage msg);

    AudioEngine& audioEngine() { return m_audioengine; }
    AudioBuilder createAudio();
    int createAudio(AudioBuilder builder);

    MidiManager& midiManager() { return m_manager; }

    void startRecording();
    void stopRecording();
    std::vector<std::pair<std::string, std::vector<MidiMessage>>> recorded();

    nlohmann::json json();
    bool load(std::string directory);
    bool save(std::string directory);

private:
    bool saveBindings(std::string filepath);
    bool saveAudio(std::string filepath);
    bool loadBindings(std::string filepath);
    bool loadAudio(std::string filepath);


    MidiManager m_manager;
    AudioEngine m_audioengine;
    MidiBindingManager m_midiBindingsManager;
    
    std::mutex m_pageMutex;
    int m_currentPage = 0;

    std::function<void(MidiDevice*, MidiMessage)> m_midiCallback;

    std::function<void(int)> m_pageChangedCallback;
};