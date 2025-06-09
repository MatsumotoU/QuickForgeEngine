#pragma once
#include "Utility/SimpleJson.h"

// TODO: jsonからpresetを読み込む

class AmbientSoundPresetData final {
public:
	AmbientSoundPresetData();
	~AmbientSoundPresetData();

public:
	void LoadPreset(const std::string& fileName);

private:
	nlohmann::json presetData_; 
};