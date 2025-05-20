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

    AudioEngine engine;

    Sound sound(pathLongMpeg, 16.f, 30.f);

    App app;
    app.addMidiBinding("Novation Launchpad Pro", MidiMessage::NoteOn, 0x60, [&sound]() {
        sound.play();
    });

    app.midiManager().refresh();

    std::cin.get();


    cleanupAudioEngine();

    return 0;
}