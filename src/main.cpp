#include <iostream>

#include <spdlog/spdlog.h>
#include <MidiManager.h>
#include <core/audioengine.h>


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




    MidiManager midiManager;
    midiManager.setMidiCallback([&sound](std::shared_ptr<MidiDevice> device, MidiMessage msg) {
        if (device->name() == "Novation Launchpad Pro") {
            if (msg.type() == MidiMessage::NoteOn)
                sound.play();
        }
    });

    try {
        midiManager.refresh();

    } catch (const RtMidiError &error) {
        std::cerr << "MIDI Error: " << error.getMessage() << "\n";
        return 1;
    }

    std::cin.get();


    cleanupAudioEngine();

    return 0;
}