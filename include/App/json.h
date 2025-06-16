#pragma once
#include <nlohmann/json.hpp>
#include "types.h"

void to_json(nlohmann::json& j, const MidiBinding& binding);
void from_json(const nlohmann::json& j, MidiBinding& binding);