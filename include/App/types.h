#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <Midi/midimessage.h>

#include <nlohmann/json.hpp>

class App;

struct MidiBinding {
    int id;
    std::string deviceName;
    MidiMessage::Type eventType;
    int key;
    std::vector<std::function<void(App&)>> actions;
    int audioId;
    int toPage;
    int onPage;

    nlohmann::json to_json() const {
        nlohmann::json j = nlohmann::json{
            {"id", id},
            {"deviceName", deviceName},
            {"eventType", eventType},
            {"key", key},
            // {"actions", actions},
            {"audioId", audioId},
            {"toPage", toPage},
            {"onPage", onPage}
        };

        return j;
    }

    void from_json(const nlohmann::json& j) {
        j.at("id").get_to<int>(id);
        j.at("deviceName").get_to<std::string>(deviceName);
        j.at("eventType").get_to<MidiMessage::Type>(eventType);
        j.at("key").get_to<int>(key);
        // j.at("actions").get_to(actions);
        j.at("audioId").get_to<int>(audioId);
        j.at("toPage").get_to<int>(toPage);
        j.at("onPage").get_to<int>(onPage);
    }
};