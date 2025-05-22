#include <iostream>

#include <spdlog/spdlog.h>

#include "App/App.h"
#include "Audio/audiomanager.h"

int main() {
    spdlog::set_level(spdlog::level::debug);


    std::string pathWav = "/home/jay/Downloads/pickupCoin.wav";
    std::string pathTwoWav = "/home/jay/Downloads/laserShoot.wav";
    std::string pathThreeWav = "/home/jay/Downloads/powerUp.wav";
    std::string pathOgg = "/home/jay/Downloads/pickupCoin.ogg";
    std::string pathMpeg = "/home/jay/Downloads/pickupCoin.mp3";
    std::string pathLongMpeg = "/home/jay/Downloads/Pumpkin  - C418.mp3";

    setupAudioEngine();

    AudioManager& manager = AudioManager::getInstance();

    int idWav = manager.loadFromFile(pathWav);
    manager.loadFromFile(pathTwoWav);
    manager.loadFromFile(pathThreeWav);
    manager.loadFromFile(pathOgg);
    manager.loadFromFile(pathMpeg);
    int idLongMpeg = manager.loadFromFile(pathLongMpeg);


    App app;
    AudioEngine engine;
    {
        std::shared_ptr<Sound> fullSoundOne = manager.getSound(idWav);
        std::shared_ptr<Sound> fullSoundTwo = manager.getSound(idLongMpeg);

        std::shared_ptr<Sound> clip = fullSoundTwo->createClip(10, 1);
        std::shared_ptr<Sound> clipTwo = fullSoundTwo->createClip(11, 1);
        std::shared_ptr<Sound> clipThree = fullSoundTwo->createClip(12, 1);

        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x62, clip);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x61, clipTwo);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x60, clipThree);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5F, fullSoundOne->clone(), 1);
        app.addMidiSound("Novation Launchpad Pro", MidiMessage::NoteOn, 0x60, fullSoundOne, 1);
        app.addMidiBinding("Novation Launchpad Pro", MidiMessage::NoteOn, 0x5E, [](App& app) {
            app.setCurrentPage(1);
        });
    }

    app.midiManager().refresh();

    std::cin.get();


    cleanupAudioEngine();

    return 0;
}