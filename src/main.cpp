#include <iostream>

#include <spdlog/spdlog.h>

#include "App/App.h"
// #include "Audio/Audio_Includes.h"

int main() {
    spdlog::enable_backtrace(100);


    std::string pathWav = "/home/jay/Downloads/pickupCoin.wav";
    std::string pathTwoWav = "/home/jay/Downloads/laserShoot.wav";
    std::string pathThreeWav = "/home/jay/Downloads/powerUp.wav";
    std::string pathOgg = "/home/jay/Downloads/pickupCoin.ogg";
    std::string pathMpeg = "/home/jay/Downloads/pickupCoin.mp3";
    std::string pathLongMpeg = "/home/jay/Downloads/Pumpkin  - C418.mp3";


    App app;
    app.setMidiCallback([](MidiDevice* device, MidiMessage msg) {
        std::cout << device->name() << ": " << std::hex << msg.key << std::endl;
    });

    AudioBuilder builderOne;
    builderOne.set_file(pathWav);
    AudioBuilder builderTwo;
    builderTwo.set_file(pathTwoWav);
    int idOne = app.audioEngine().create(builderOne);
    int idTwo = app.audioEngine().create(builderTwo);

    app.setCurrentPage(0);

    auto binding = MidiBindingBuilder("Novation Launchpad Pro").audio(idOne).key(0x5E).type(MidiMessage::NoteOn).on_page(0).build();
    auto pageBinding = MidiBindingBuilder("Novation Launchpad Pro").audio(idTwo).key(0x5F).type(MidiMessage::NoteOn).on_page(0).change_page_to(1).build();
    auto backBinding = MidiBindingBuilder("Novation Launchpad Pro").key(0x5F).type(MidiMessage::NoteOn).on_page(1).change_page_to(0).build();
    app.addMidiBinding(binding);
    app.addMidiBinding(pageBinding);
    app.addMidiBinding(backBinding);

    
    app.midiManager().refresh();
    
    app.json();

    app.save("/home/jay/Desktop/TestingAudio");

    app.load("/home/jay/Desktop/TestingAudio");

    std::cin.get();

    spdlog::dump_backtrace();

    return 0;
}