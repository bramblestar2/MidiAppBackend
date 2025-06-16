#include "App/MidiBindingBuilder.h"
#include "App/types.h"

MidiBindingBuilder::MidiBindingBuilder(const std::string& deviceName)
    : m_deviceName(deviceName)
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


MidiBinding MidiBindingBuilder::build() {
    MidiBinding binding;
    binding.actions = std::move(this->m_actions);
    binding.audioId = this->m_audioId;
    binding.deviceName = this->m_deviceName;
    binding.eventType = this->m_type;
    binding.key = this->m_key;
    binding.toPage = this->m_toPage;
    binding.onPage = this->m_onPage;

    return binding;
}