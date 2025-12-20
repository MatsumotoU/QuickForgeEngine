#include "engine/include/input/KeyConfig.h"
#include <cassert>
#include <dinput.h>
#include <nlohmann/json.hpp>

#include <fstream>

#include "engine/include/assets/AssetManager.h"

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void KeyConfig::Initialize() {
	keyMap_.clear();
	SettingDefaultKeyConfig();
}

void KeyConfig::AddKey(const std::string& name, uint32_t key) {
	// 蜷後§繧ｭ繝ｼ縺檎匳骭ｲ縺輔ｌ縺ｦ縺・↑縺・°遒ｺ隱・
	for (const auto& existingKey : keyMap_[name]) {
		if (existingKey == key) {
			return;
		}
	}
	keyMap_[name].push_back(key);
}

void KeyConfig::RemoveKey(const std::string& name) {
	keyMap_.erase(name);
}

void KeyConfig::RemoveKey(const std::string& name, uint32_t key) {
	auto it = keyMap_.find(name);
	if (it != keyMap_.end()) {
		auto& keys = it->second;
		keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
		if (keys.empty()) {
			keyMap_.erase(it);
		}
	}
}

void KeyConfig::EditKey(const std::string& name, size_t index, uint32_t newKey) {
	auto it = keyMap_.find(name);
	if (it != keyMap_.end() && index < it->second.size()) {
		it->second[index] = newKey;
	} else {
		AddKey(name, newKey);
	}
}

const std::vector<uint32_t>& KeyConfig::GetKeys(const std::string& name) const {
	static const std::vector<uint32_t> emptyVector;
	auto it = keyMap_.find(name);
	if (it != keyMap_.end()) {
		return it->second;
	}
	
#ifdef _DEBUG
	DebugLog("GetKeys: No keys found for action '" + name + "'", LogLevel::Error);
#endif // _DEBUG
	return emptyVector;
}

void KeyConfig::SettingDefaultKeyConfig() {
	// 遘ｻ蜍・
	AddKey("MoveRight", DIK_RIGHT);
	AddKey("MoveRight", DIK_D);
	AddKey("MoveLeft", DIK_LEFT);
	AddKey("MoveLeft", DIK_A);
	AddKey("MoveUp", DIK_UP);
	AddKey("MoveUp", DIK_W);
	AddKey("MoveDown", DIK_DOWN);
	AddKey("MoveDown", DIK_S);
	// 繧ｸ繝｣繝ｳ繝・
	AddKey("Jump", DIK_SPACE);
	// 謾ｻ謦・
	AddKey("Attack", DIK_LCONTROL);
	// 繝｡繝九Η繝ｼ
	AddKey("Menu", DIK_ESCAPE);
	// 豎ｺ螳・
	AddKey("Decide", DIK_RETURN);
	AddKey("Decide", DIK_SPACE);
	// 繧ｭ繝｣繝ｳ繧ｻ繝ｫ
	AddKey("Cancel", DIK_BACKSPACE);
	AddKey("Cancel", DIK_ESCAPE);
}

void KeyConfig::Finalize() {
	keyMap_.clear();
}

void KeyConfig::Reset() {
	keyMap_.clear();
	SettingDefaultKeyConfig();
}

void KeyConfig::SaveKeyConfig() {
	try {
		std::string path = 
			AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Config") 
			+ "KeyConfig.json";
		nlohmann::json keyConfigJson;
		for (const auto& [actionName, keys] : keyMap_) {
			keyConfigJson[actionName] = keys;
		}
		std::ofstream ofs(path);
		ofs << keyConfigJson.dump(4);
		ofs.close();
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);
#endif // _DEBUG
	}
}

void KeyConfig::LoadKeyConfig() {
	try {
		std::string path =
			AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Config")
			+ "KeyConfig.json";
		std::ifstream ifs(path);
		nlohmann::json keyConfigJson;
		if (ifs.is_open()) {
			ifs >> keyConfigJson;
			ifs.close();
		}
		keyMap_.clear();
		if (keyConfigJson.size() == 0) {
			SettingDefaultKeyConfig();
			return;
		}
		for (auto& [actionName, keys] : keyConfigJson.items()) {
			keyMap_[actionName] = keys.get<std::vector<uint32_t>>();
		}
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);
#endif // _DEBUG
	}
}
