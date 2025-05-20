#include "App.h"

App::App() {
    m_manager.setMidiCallback([this](std::shared_ptr<MidiDevice> device, MidiMessage msg) {
        this->handleMidiMessage(device, msg);
    });
}


int App::addMidiBinding(std::string deviceName, MidiMessage::Type eventType, int key, std::function<void()> action, int page) {
    int id = _acquireID();
    
    {
        m_midiBindingsPages[page].emplace_back(
            id,
            deviceName,
            eventType,
            key,
            std::move(action)
        );
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
            // release all removed IDs back to pool
            for (auto itr = it; itr != bindings.end(); ++itr) {
                _releaseID(itr->id);
            }
            bindings.erase(it, bindings.end());
        }
    }
}


const std::vector<App::MidiBinding>& App::getMidiBindingsForPage(int page) const {
    static const std::vector<MidiBinding> empty;

    std::lock_guard<std::mutex> lock(m_bindingsMutex);
    auto it = m_midiBindingsPages.find(page);

    return (it != m_midiBindingsPages.end()) ? it->second : empty;
}


void App::handleMidiMessage(std::shared_ptr<MidiDevice> device, MidiMessage msg) {
    const std::string name = device->name();
    std::lock_guard<std::mutex> lock(m_bindingsMutex);

    for (auto& [page, bindings] : m_midiBindingsPages) {
        for (auto& b : bindings) {
            if (b.deviceName == name &&
                b.eventType == msg.type() &&
                b.key == msg.key) {
                b.action();
            }
        }
    }    
}



int App::_acquireID() {
    std::lock_guard<std::mutex> lock(m_idMutex);

    if (!m_freeIDs.empty()) {
        int id = m_freeIDs.front();
        m_freeIDs.pop();
        return id;
    }

    return m_nextID++;
}


void App::_releaseID(int id) {
    std::lock_guard<std::mutex> lock(m_idMutex);
    m_freeIDs.push(id);
}