#include <iostream>

#include <spdlog/spdlog.h>

#include "App/App.h"
#include "Audio/Audio_Includes.h"

int main() {
    spdlog::enable_backtrace(100);


    std::string pathWav = "/home/jay/Downloads/pickupCoin.wav";
    std::string pathTwoWav = "/home/jay/Downloads/laserShoot.wav";
    std::string pathThreeWav = "/home/jay/Downloads/powerUp.wav";
    std::string pathOgg = "/home/jay/Downloads/pickupCoin.ogg";
    std::string pathMpeg = "/home/jay/Downloads/pickupCoin.mp3";
    std::string pathLongMpeg = "/home/jay/Downloads/Pumpkin  - C418.mp3";

    setupAudioEngine();

    App app;

    int idOne = app.createAudio(pathLongMpeg).start(20.0).end(21.0).build();
    int idTwo = app.createAudio(pathLongMpeg).start(21.0).end(22.0).build();
    int idThree = app.createAudio(pathLongMpeg).start(22.0).end(23.0).build();
    int idFour = app.createAudio(pathLongMpeg).start(23.0).end(24.0).build();

    {
        app.midiBind("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5E).on_page(0).change_page_to(1);

        app.midiBind("Novation Launchpad Pro", MidiMessage::NoteOn, 0x62).audio(idOne);
        app.midiBind("Novation Launchpad Pro", MidiMessage::NoteOn, 0x61).audio(idTwo);
        app.midiBind("Novation Launchpad Pro", MidiMessage::NoteOn, 0x60).audio(idThree);
        app.midiBind("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5F).on_page(1).audio(idThree);

        // app.addMidiBinding("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5E, [](App& app) {
        //     app.setCurrentPage(1);
        // });
    }

    app.midiManager().refresh();

    std::cin.get();


    cleanupAudioEngine();

    spdlog::dump_backtrace();

    return 0;
}