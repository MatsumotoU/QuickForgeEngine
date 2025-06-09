#include "AmbientSoundPresetData.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

AmbientSoundPresetData::AmbientSoundPresetData() {
	presetData_.clear();
}

AmbientSoundPresetData::~AmbientSoundPresetData() {
	presetData_.clear();
}

void AmbientSoundPresetData::LoadPreset(const std::string& fileName) {
	presetData_ = SJN::LoadJsonData(fileName);
	if (presetData_.is_null()) {
		
	}
}
