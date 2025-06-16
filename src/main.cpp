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

    AudioBuilder builder;
    builder.set_file(pathWav);
    int idOne = app.audioEngine().create(builder);

    app.setCurrentPage(0);
    // auto audioOne = app.createAudio(pathLongMpeg).set_start(21.0).set_end(22.0).build();
    // auto audioTwo = app.createAudio(pathLongMpeg).set_start(22.0).set_end(23.0).build();
    // auto audioThree = app.createAudio(pathLongMpeg).set_start(23.0).set_end(30.0).build();

    // int idTwo = app.audioEngine().add(audioOne);
    // int idThree = app.audioEngine().add(audioTwo);
    // int idFour = app.audioEngine().add(audioThree);

    auto binding = MidiBindingBuilder("Novation Launchpad Pro").audio(idOne).key(0x5E).type(MidiMessage::NoteOn).on_page(0).build();
    app.addMidiBinding(binding);

    
    app.midiManager().refresh();
    
    app.json();

    std::cin.get();

    spdlog::dump_backtrace();

    return 0;
}