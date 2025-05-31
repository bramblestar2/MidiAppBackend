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

    app.setCurrentPage(0);
    auto audio = app.createAudio(pathWav).set_filepath(pathWav).build();
    // auto audioOne = app.createAudio(pathLongMpeg).set_start(21.0).set_end(22.0).build();
    // auto audioTwo = app.createAudio(pathLongMpeg).set_start(22.0).set_end(23.0).build();
    // auto audioThree = app.createAudio(pathLongMpeg).set_start(23.0).set_end(30.0).build();

    int idOne = app.audioEngine().add(audio);
    // int idTwo = app.audioEngine().add(audioOne);
    // int idThree = app.audioEngine().add(audioTwo);
    // int idFour = app.audioEngine().add(audioThree);

    // {
        app.midiBind("Novation Launchpad Pro").audio(idOne).key(0x5E).type(MidiMessage::NoteOn).on_page(0).build();

    //     app.midiBind("Novation Launchpad Pro").key(0x62).type(MidiMessage::NoteOn).audio(idOne).build();
    //     app.midiBind("Novation Launchpad Pro").key(0x61).type(MidiMessage::NoteOn).audio(idTwo).build();
    //     app.midiBind("Novation Launchpad Pro").key(0x60).type(MidiMessage::NoteOn).audio(idThree).build();
    //     app.midiBind("Novation Launchpad Pro").key(0x5F).type(MidiMessage::NoteOn).on_page(1).audio(idFour).build();

    //     // app.addMidiBinding("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5E, [](App& app) {
    //     //     app.setCurrentPage(1);
    //     // });
    // }

    
    app.midiManager().refresh();
    
    std::cin.get();

    spdlog::dump_backtrace();

    return 0;
}