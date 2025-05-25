#include "App.h"


App::App() {
    m_manager.setMidiCallback([this](std::shared_ptr<MidiDevice> device, MidiMessage msg) {
        this->handleMidiMessage(device, msg);
    });
}


void App::setCurrentPage(const int page) { 
    std::lock_guard<std::mutex> lock(m_pageMutex);
    m_currentPage = page; 
}


int App::addMidiBinding(std::string deviceName, MidiMessage::Type eventType, int key, std::function<void(App&)> action, int page) {
    std::lock_guard<std::mutex> lock(m_bindingsMutex);
    int id = m_idPool.acquire();
    
    {
        std::vector<std::function<void(App&)>> list;
        list.push_back(std::move(action));

        m_midiBindingsPages[page].emplace_back(
            id,
            deviceName,
            eventType,
            key,
            list,
            std::nullopt
        );
    }

    spdlog::debug("Added MIDI binding: ID={}, Device='{}', Event={}, Key={}, Page={}", 
        id, deviceName, static_cast<int>(eventType), key, page);
    
    return id;
}


int App::addMidiSound(std::string deviceName, MidiMessage::Type eventType, int key, int audio_id, int page) {
    std::lock_guard<std::mutex> lock(m_bindingsMutex);
    int id = m_idPool.acquire();
    
    {
        std::vector<std::function<void(App&)>> list;
        m_midiBindingsPages[page].emplace_back(
            id,
            deviceName,
            eventType,
            key,
            list
        );

        MidiBinding& binding_ref = m_midiBindingsPages[page].back();
        binding_ref.audio = m_engine.get(audio_id);

        binding_ref.actions.push_back([this, audio_id](App&) {
            m_engine.play(audio_id);
        });
    }

    spdlog::debug("Added MIDI binding: ID={}, Device='{}', Event={}, Key={}, Page={}", 
        id, deviceName, static_cast<int>(eventType), key, page);

    return id;
}


void App::removeMidiBinding(const int id) {
    std::lock_guard<std::mutex> lock(m_bindingsMutex);

    for (auto& [page, bindings] : m_midiBindingsPages) {
        auto it = std::remove_if(bindings.begin(), bindings.end(),
            [this, id](const MidiBinding& b) {
                return b.id == id;
            });
        if (it != bindings.end()) {
            for (auto itr = it; itr != bindings.end(); ++itr) {
                m_idPool.release(itr->id);
            }
            bindings.erase(it, bindings.end());
        }
    }
}


const std::vector<App::MidiBinding>& App::getMidiBindingsForPage(int page) const {
    static const std::vector<MidiBinding> empty;

    auto it = m_midiBindingsPages.find(page);

    return (it != m_midiBindingsPages.end()) ? it->second : empty;
}


void App::handleMidiMessage(std::shared_ptr<MidiDevice> device, MidiMessage msg) {
    const std::string name = device->name();
    std::lock_guard<std::mutex> lock(m_bindingsMutex);

    if (m_midiBindingsPages.count(m_currentPage) > 0) {
        for (auto& b : m_midiBindingsPages.at(m_currentPage)) {
            if (b.deviceName == name &&
                b.eventType == msg.type() &&
                b.key == msg.key) {
                for (auto& action : b.actions) {
                    action(*this);
                }
            }
        }
    } 
}




MidiBindingBuilder::MidiBindingBuilder(App& app, const std::string& deviceName, MidiMessage::Type type, int key)
    : m_app(app), m_deviceName(deviceName), m_type(type), m_key(key)
{
}


MidiBindingBuilder& MidiBindingBuilder::toSound(int audio_id, int page) {
    m_app.addMidiSound(m_deviceName, m_type, m_key, audio_id, page);
    return *this;
}


MidiBindingBuilder& MidiBindingBuilder::toAction(std::function<void(App&)> callback) {
    m_app.addMidiBinding(m_deviceName, m_type, m_key, std::move(callback));
    return *this;
}