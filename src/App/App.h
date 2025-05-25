#pragma once
#include <map>

#include <spdlog/spdlog.h>
#include <Midi/Midi.h>
#include <Audio/Audio_Includes.h>

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
        std::vector<std::function<void(App&)>> actions;
        Audio* audio;
        int audioId;
    };

public:
    App();

    void setCurrentPage(const int page);
    const int& getCurrentPage() const { return m_currentPage; }

    //returns binding ID
    int addMidiBinding(std::string deviceName, MidiMessage::Type eventType, int key, std::function<void(App&)> action, int page = 0);
    int addMidiSound(std::string deviceName, MidiMessage::Type eventType, int key, int audio_id, int page = 0);
    void removeMidiBinding(const int id);
    const std::map<int, std::vector<MidiBinding>>& getMidiBindingPages() const { return m_midiBindingsPages; }
    const std::vector<MidiBinding>& getMidiBindingsForPage(int page) const;


    AudioBuilder create_audio(const std::string &filepath);


    MidiManager& midiManager() { return m_manager; }
    AudioEngine& audioEngine() { return m_engine; }

private:
    void handleMidiMessage(std::shared_ptr<MidiDevice> device, MidiMessage msg);


private:
    MidiManager m_manager;
    AudioEngine m_engine;

    mutable std::mutex m_bindingsMutex;
    std::map<int, std::vector<MidiBinding>> m_midiBindingsPages;

    IdPool m_idPool;
    
    std::mutex m_pageMutex;
    int m_currentPage = 0;
};


class MidiBindingBuilder {
public:
    MidiBindingBuilder(App& app, const std::string& deviceName, MidiMessage::Type type, int key);

    MidiBindingBuilder& toSound(int audio_id, int page = 0);
    MidiBindingBuilder& toAction(std::function<void(App&)> callback);

private:
    App& m_app;
    std::string m_deviceName;
    MidiMessage::Type m_type;
    int m_key;
};