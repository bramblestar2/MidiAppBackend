#pragma once
#include <map>

#include <spdlog/spdlog.h>
#include <Midi/Midi.h>
#include <Audio/audioengine.h>
#include <functional>

#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <optional>
#include <memory>

class App {
private:
    struct MidiBinding {
        int id;
        std::string deviceName;
        MidiMessage::Type eventType;
        int key;
        std::function<void(App&)> action;
        std::optional<std::unique_ptr<Sound>> sound;
    };

public:
    App();

    void setCurrentPage(const int page);
    const int& getCurrentPage() const { return m_currentPage; }

    //returns binding ID
    int addMidiBinding(std::string deviceName, MidiMessage::Type eventType, int key, std::function<void(App&)> action, int page = 0);
    int addMidiSound(std::string deviceName, MidiMessage::Type eventType, int key, std::unique_ptr<Sound>& sound, int page = 0);
    void removeMidiBinding(const int id);
    const std::map<int, std::vector<MidiBinding>>& getMidiBindingPages() const { return m_midiBindingsPages; }
    const std::vector<MidiBinding>& getMidiBindingsForPage(int page) const;

    MidiManager& midiManager() { return m_manager; }
    AudioEngine& audioEngine() { return m_engine; }

private:
    void handleMidiMessage(std::shared_ptr<MidiDevice> device, MidiMessage msg);
    int _acquireID();
    void _releaseID(int id);


private:
    MidiManager m_manager;
    AudioEngine m_engine;

    mutable std::mutex m_bindingsMutex;
    std::map<int, std::vector<MidiBinding>> m_midiBindingsPages;

    mutable std::mutex m_idMutex;
    std::queue<int> m_freeIDs;
    uint32_t m_nextID = 0;
    
    std::mutex m_pageMutex;
    int m_currentPage = 0;
};