#pragma once

#include <mutex>
#include <map>
#include <functional>
#include <Midi/Midi.h>

#include "IdPool.h"
#include "types.h"
#include "MidiBindingBuilder.h"

class App;

class MidiBindingManager {
public:
    void handleMidiMessage(App* app, MidiDevice* device, MidiMessage msg);

    //returns binding ID
    int addMidiBinding(MidiBinding binding);
    void removeMidiBinding(const int id);

    const std::map<int, std::vector<MidiBinding>>& getPages() const;
    std::vector<MidiBinding*> getBindingsForPage(int page);
    std::vector<MidiBinding*> getMidiBindings();

    MidiBindingBuilder midiBind(const std::string &deviceName);

    void onMidiBindingsChanged(std::function<void()> callback);
    
private:
    mutable std::mutex m_bindingsMutex;
    std::map<int, std::vector<MidiBinding>> m_midiBindingsPages;
    IdPool m_idPool;
    
    std::function<void()> m_midiBindingsChangedCallback;
};