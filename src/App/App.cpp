#include "App/App.h"
#include <fstream>
#include <algorithm>
#include "App/json.h"


App::App() {
    m_manager.setMidiCallback([this](MidiDevice* device, MidiMessage msg) {
        this->handleMidiMessage(device, msg);

        if (this->m_midiCallback) this->m_midiCallback(device, msg);
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
    m_midiBindingsManager.onMidiBindingsChanged(callback);
}


void App::onAudioListChanged(std::function<void()> callback)
{
    m_audioengine.onAudioListChanged(std::move(callback));
}


int App::addMidiBinding(MidiBinding binding) 
{
    return m_midiBindingsManager.addMidiBinding(binding);
}

void App::removeMidiBinding(const int id) 
{
    m_midiBindingsManager.removeMidiBinding(id);
}


const std::map<int, std::vector<MidiBinding>> &App::getMidiBindingPages() const
{
    return m_midiBindingsManager.getPages();
}


std::vector<MidiBinding*> App::getMidiBindingsForPage(int page) 
{
    return m_midiBindingsManager.getBindingsForPage(page);
}


MidiBindingBuilder App::midiBind(const std::string &deviceName)
{
    return MidiBindingBuilder(deviceName);
}


std::vector<MidiBinding *> App::getMidiBindings()
{
    return m_midiBindingsManager.getMidiBindings();
}


void App::handleMidiMessage(MidiDevice* device, MidiMessage msg) {
    m_midiBindingsManager.handleMidiMessage(this, device, msg);
}


nlohmann::json App::json() {
    std::map<std::string, std::vector<MidiBinding>> pages;

    std::transform(m_midiBindingsManager.getPages().begin(), m_midiBindingsManager.getPages().end(), std::inserter(pages, pages.begin()),
        [](const std::pair<int, std::vector<MidiBinding>>& page) {
            return std::make_pair(std::to_string(page.first), page.second);
        });

    nlohmann::json j = {
        { "bindings", pages },
        { "audio", nlohmann::json::parse(m_audioengine.json()) }
    };

    return j.dump();
}


bool App::load(std::string filepath) {
    return false;
}


bool App::save(std::string filepath) {
    return false;
}