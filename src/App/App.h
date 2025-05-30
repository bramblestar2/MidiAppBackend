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


class App;
class MidiBindingBuilder;

class App {
public:
    struct MidiBinding {
        int id;
        std::string deviceName;
        MidiMessage::Type eventType;
        int key;
        std::vector<std::function<void(App&)>> actions;
        Audio* audio;
        int audioId;
        int toPage;
        int onPage;
    };

public:
    App();

    void setCurrentPage(const int page);
    const int& getCurrentPage() const { return m_currentPage; }

    void setMidiCallback(std::function<void(MidiDevice*, MidiMessage)> &&callback);
    void onMidiBindingsChanged(std::function<void()> callback);

    //returns binding ID
    int addMidiBinding(MidiBinding&& binding);
    int addMidiSound(std::string deviceName, MidiMessage::Type eventType, int key, int audio_id, int page = 0);
    void removeMidiBinding(const int id);
    const std::map<int, std::vector<MidiBinding>>& getMidiBindingPages() const { return m_midiBindingsPages; }
    const std::vector<MidiBinding>& getMidiBindingsForPage(int page) const;

    MidiBindingBuilder midiBind(const std::string &deviceName);

    std::vector<App::MidiBinding*> getMidiBindings();

    AudioBuilder createAudio(const std::string &filepath);

    MidiManager& midiManager() { return m_manager; }
    AudioEngine& audioEngine() { return m_engine; }

private:
    void handleMidiMessage(MidiDevice* device, MidiMessage msg);

    MidiManager m_manager;
    AudioEngine m_engine;

    mutable std::mutex m_bindingsMutex;
    std::map<int, std::vector<MidiBinding>> m_midiBindingsPages;

    IdPool m_idPool;
    
    std::mutex m_pageMutex;
    int m_currentPage = 0;

    std::function<void(MidiDevice*, MidiMessage)> m_midiCallback;
    std::function<void()> m_midiBindingsChangedCallback;
};




class MidiBindingBuilder {
public:
    MidiBindingBuilder(App& app, const std::string& deviceName);

    MidiBindingBuilder& audio(int audio_id);
    MidiBindingBuilder& action(std::function<void(App&)> callback);
    MidiBindingBuilder& on_page(int page);
    MidiBindingBuilder& change_page_to(int page);
    MidiBindingBuilder& key(int key);
    MidiBindingBuilder& type(MidiMessage::Type type);

    void build();

private:
    App& m_app;
    std::string m_deviceName;
    MidiMessage::Type m_type = MidiMessage::Type::UNKNOWN;
    int m_key = 0;
    int m_onPage = 0;
    int m_toPage = 0;
    int m_audioId = 0;
    std::vector<std::function<void(App&)>> m_actions;
};