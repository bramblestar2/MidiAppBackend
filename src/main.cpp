#include <iostream>

#include <spdlog/spdlog.h>

#include "App/App.h"
#include "Audio/Audio_Includes.h"

int main() {
    spdlog::set_level(spdlog::level::debug);


    std::string pathWav = "/home/jay/Downloads/pickupCoin.wav";
    std::string pathTwoWav = "/home/jay/Downloads/laserShoot.wav";
    std::string pathThreeWav = "/home/jay/Downloads/powerUp.wav";
    std::string pathOgg = "/home/jay/Downloads/pickupCoin.ogg";
    std::string pathMpeg = "/home/jay/Downloads/pickupCoin.mp3";
    std::string pathLongMpeg = "/home/jay/Downloads/Pumpkin  - C418.mp3";

    setupAudioEngine();

    App app;

    int idOne = app.audioEngine().create_audio(pathLongMpeg).start(20.0).end(21.0).build();
    int idTwo = app.audioEngine().create_audio(pathLongMpeg).start(21.0).end(22.0).build();
    int idThree = app.audioEngine().create_audio(pathLongMpeg).start(22.0).end(23.0).build();
    int idFour = app.audioEngine().create_audio(pathLongMpeg).start(23.0).end(24.0).build();

    {
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x62, idOne);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x61, idTwo);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x60, idThree);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5F, idFour, 1);
        app.addMidiBinding("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5E, [](App& app) {
            app.setCurrentPage(1);
        });
    }

    app.midiManager().refresh();

    std::cin.get();


    cleanupAudioEngine();

    return 0;
}