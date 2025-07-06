#include "App/MidiBindingManager.h"
#include "App/App.h"

#include <algorithm>


void MidiBindingManager::handleMidiMessage(App* app, MidiDevice* device, MidiMessage msg) {
    const std::string name = device->name();
    std::lock_guard<std::mutex> lock(m_bindingsMutex);

    if (m_midiBindingsPages.count(app->getCurrentPage()) > 0) {
        int page = app->getCurrentPage();

        for (auto& b : m_midiBindingsPages.at(page)) {
            int key = msg[1];

            if (b.deviceName == name &&
                b.eventType == msg.get_message_type() &&
                b.key == key) {
                for (auto& action : b.actions) {
                    action(*app);
                }

                if (b.toPage != -1) {
                    app->setCurrentPage(b.toPage);
                }

                app->audioEngine().play(b.audioId);
            }
        }
    } 
}


void MidiBindingManager::setMidiBindings(std::map<int, std::vector<MidiBinding>> bindings)
{
    std::lock_guard<std::mutex> lock(m_bindingsMutex);
    m_midiBindingsPages = bindings;
}


int MidiBindingManager::addMidiBinding(MidiBinding binding) {
    std::lock_guard<std::mutex> lock(m_bindingsMutex);
    int id = m_idPool.acquire();
    
    {
        spdlog::debug("Added MIDI binding: ID={}, Device='{}', Event={}, Key={}, Page={}", 
            id, binding.deviceName, static_cast<int>(binding.eventType), binding.key, binding.onPage);

        binding.id = id;
        m_midiBindingsPages[binding.onPage].emplace_back(std::move(binding));
    }
    
    if (m_midiBindingsChangedCallback) this->m_midiBindingsChangedCallback();

    return id;
}


void MidiBindingManager::removeMidiBinding(int id) {
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


const std::map<int, std::vector<MidiBinding>>& MidiBindingManager::getPages() const 
{ 
    return m_midiBindingsPages; 
}


std::vector<MidiBinding*> MidiBindingManager::getBindingsForPage(int page) {
    std::vector<MidiBinding*> result;
    
    auto it = m_midiBindingsPages.find(page);

    std::transform(it->second.begin(), it->second.end(), std::back_inserter(result),
        [](MidiBinding& b) {
            return &b;
        });

    return result;
}


std::vector<MidiBinding*> MidiBindingManager::getMidiBindings() {
    std::vector<MidiBinding*> result;
    for (auto& [page, bindings] : m_midiBindingsPages) {
        std::transform(bindings.begin(), bindings.end(), std::back_inserter(result),
            [](MidiBinding& b) {
                return &b;
            });
    }
    return result;
}


MidiBindingBuilder MidiBindingManager::midiBind(const std::string &deviceName) {
    return MidiBindingBuilder(deviceName);
}


void MidiBindingManager::onMidiBindingsChanged(std::function<void()> callback) {
    m_midiBindingsChangedCallback = callback;
}