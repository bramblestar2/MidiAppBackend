#include <iostream>

#include <spdlog/spdlog.h>

#include "App/App.h"

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
    AudioEngine engine;
    {
        std::unique_ptr<Sound> soundOne(new Sound(pathLongMpeg, 16.f, 18.f));
        std::unique_ptr<Sound> soundTwo(new Sound(pathLongMpeg, 18.f, 20.f));
        std::unique_ptr<Sound> soundThree(new Sound(pathLongMpeg, 20.f, 22.f));
        
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x61, soundOne);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x60, soundTwo);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5F, soundThree, 1);
        app.addMidiBinding("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5E, [](App& app) {
            app.setCurrentPage(1);
        });
    }

    app.midiManager().refresh();

    std::cin.get();


    cleanupAudioEngine();

    return 0;
}