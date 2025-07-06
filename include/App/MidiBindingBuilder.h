#pragma once

#include <vector>
#include <string>
#include <functional>
#include "App/types.h"


class App;

class MidiBindingBuilder {
public:
    MidiBindingBuilder(const std::string& deviceName);

    MidiBindingBuilder& audio(int audio_id);
    MidiBindingBuilder& action(std::function<void(App&)> callback);
    MidiBindingBuilder& on_page(int page);
    MidiBindingBuilder& change_page_to(int page);
    MidiBindingBuilder& key(int key);
    MidiBindingBuilder& type(libremidi::message_type type);

    MidiBinding build();

private:
    std::string m_deviceName;
    libremidi::message_type m_type{libremidi::message_type::INVALID};
    int m_key = 0;
    int m_onPage = 0;
    int m_toPage = 0;
    int m_audioId = 0;
    std::vector<std::function<void(App&)>> m_actions;
};