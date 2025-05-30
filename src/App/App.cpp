#include "App.h"


App::App() {
    m_manager.setMidiCallback([this](MidiDevice* device, MidiMessage msg) {
        this->handleMidiMessage(device, msg);
        this->m_midiCallback(device, msg);
    });
}


void App::setCurrentPage(const int page) { 
    std::lock_guard<std::mutex> lock(m_pageMutex);
    m_currentPage = page; 
}


void App::setMidiCallback(std::function<void(MidiDevice*, MidiMessage)> &&callback) {
    m_midiCallback = std::move(callback);
}

void App::onMidiBindingsChanged(std::function<void()> callback)
{
    m_midiBindingsChangedCallback = std::move(callback);
}

int App::addMidiBinding(MidiBinding&& binding) {
    std::lock_guard<std::mutex> lock(m_bindingsMutex);
    int id = m_idPool.acquire();
    
    {
        // std::vector<std::function<void(App&)>> list;
        // list.push_back(std::move(action));

        // m_midiBindingsPages[page].emplace_back(
        //     id,
        //     deviceName,
        //     eventType,
        //     key,
        //     list,
        //     nullptr
        // );

        spdlog::debug("Added MIDI binding: ID={}, Device='{}', Event={}, Key={}, Page={}", 
            id, binding.deviceName, static_cast<int>(binding.eventType), binding.key, binding.onPage);

        m_midiBindingsPages[binding.onPage].emplace_back(std::move(binding));
    }
    

    this->m_midiBindingsChangedCallback();

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
            list,
            nullptr,
            audio_id
        );

        MidiBinding& binding_ref = m_midiBindingsPages[page].back();
        binding_ref.audio = m_engine.get(audio_id);

        binding_ref.actions.push_back([this, audio_id](App&) {
            m_engine.play(audio_id);
        });
    }

    spdlog::debug("Added MIDI binding: ID={}, Device='{}', Event={}, Key={}, Page={}", 
        id, deviceName, static_cast<int>(eventType), key, page);

    this->m_midiBindingsChangedCallback();

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


MidiBindingBuilder App::midiBind(const std::string &deviceName)
{
    return MidiBindingBuilder(*this, deviceName);
}


std::vector<App::MidiBinding *> App::getMidiBindings()
{
    std::vector<App::MidiBinding *> result;

    for (auto& [page, bindings] : m_midiBindingsPages) {
        for (auto& binding : bindings) {
            result.emplace_back(&binding);
        }
    }

    return result;
}


void App::handleMidiMessage(MidiDevice* device, MidiMessage msg) {
    const std::string name = device->name();
    std::lock_guard<std::mutex> lock(m_bindingsMutex);

    if (m_midiBindingsPages.count(m_currentPage) > 0) {
        int page = m_currentPage;

        for (auto& b : m_midiBindingsPages.at(page)) {
            if (b.deviceName == name &&
                b.eventType == msg.type() &&
                b.key == msg.key) {
                for (auto& action : b.actions) {
                    action(*this);
                }

                if (b.toPage != -1) {
                    setCurrentPage(b.toPage);
                }

                m_engine.play(b.audioId);
            }
        }
    } 
}


AudioBuilder App::createAudio(const std::string &filepath) {
    return m_engine.create_audio(filepath);
}







MidiBindingBuilder::MidiBindingBuilder(App& app, const std::string& deviceName)
    : m_app(app), m_deviceName(deviceName)
{
}


MidiBindingBuilder& MidiBindingBuilder::audio(int audio_id) {
    m_audioId = audio_id;
    return *this;
}


MidiBindingBuilder& MidiBindingBuilder::action(std::function<void(App&)> callback) {
    this->m_actions.push_back(std::move(callback));
    return *this;
}


MidiBindingBuilder &MidiBindingBuilder::on_page(int page) {
    m_onPage = page;
    return *this;
}


MidiBindingBuilder &MidiBindingBuilder::change_page_to(int page) {
    m_toPage = page;
    return *this;
}


MidiBindingBuilder &MidiBindingBuilder::key(int key) {
    m_key = key;
    return *this;
}


MidiBindingBuilder &MidiBindingBuilder::type(MidiMessage::Type type) {
    m_type = type;
    return *this;
}


void MidiBindingBuilder::build() {
    App::MidiBinding binding;
    binding.actions = std::move(this->m_actions);
    binding.audioId = this->m_audioId;
    binding.deviceName = this->m_deviceName;
    binding.eventType = this->m_type;
    binding.key = this->m_key;
    binding.toPage = this->m_toPage;
    binding.onPage = this->m_onPage;
    m_app.addMidiBinding(std::move(binding));
}