#include "App/json.h"

void to_json(nlohmann::json& j, const MidiBinding& binding) {
    j = binding.to_json();
}

void from_json(const nlohmann::json& j, MidiBinding& binding) {
    binding.from_json(j);
}