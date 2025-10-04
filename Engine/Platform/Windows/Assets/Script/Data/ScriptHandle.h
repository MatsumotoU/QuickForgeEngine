#pragma once
#include "Core/Entity/Component/ComponentData.h"
#include <vector>
#include <unordered_map>

struct LuaHandle {
	std::string scriptName_;
	std::unordered_map<std::string, int> intParams_;
	std::unordered_map<std::string, float> floatParams_;
	std::unordered_map<std::string, bool> boolParams_;
	std::unordered_map<std::string, std::string> stringParams_;
	uint32_t handle_;
};

class ScriptHandles final :public ComponentData {
public:
	std::vector<LuaHandle> scriptHandles_;
	
	ScriptHandles() = default;
	~ScriptHandles() override = default;
	nlohmann::json Serialize() const override {
		nlohmann::json json;
		for (const auto& scriptHandle : scriptHandles_) {
			nlohmann::json shJson;
			shJson["scriptName"] = scriptHandle.scriptName_;
			shJson["handle"] = scriptHandle.handle_;
			json["scriptHandles"].push_back(shJson);

			json["intParams"] = scriptHandle.intParams_;
			json["floatParams"] = scriptHandle.floatParams_;
			json["boolParams"] = scriptHandle.boolParams_;
			json["stringParams"] = scriptHandle.stringParams_;
			
		}
		return json;
	}
	void Deserialize(const nlohmann::json& json) override {
		scriptHandles_.clear();
		if (json.contains("scriptHandles") && json["scriptHandles"].is_array()) {
			for (const auto& shJson : json["scriptHandles"]) {
				LuaHandle sh;
				if (shJson.contains("scriptName") && shJson["scriptName"].is_string()) {
					sh.scriptName_ = shJson["scriptName"].get<std::string>();
				}
				if (shJson.contains("handle") && shJson["handle"].is_number_unsigned()) {
					sh.handle_ = shJson["handle"].get<uint32_t>();
				}
				if (json.contains("intParams") && json["intParams"].is_object()) {
					sh.intParams_ = json["intParams"].get<std::unordered_map<std::string, int>>();
				}
				if (json.contains("floatParams") && json["floatParams"].is_object()) {
					sh.floatParams_ = json["floatParams"].get<std::unordered_map<std::string, float>>();
				}
				if (json.contains("boolParams") && json["boolParams"].is_object()) {
					sh.boolParams_ = json["boolParams"].get<std::unordered_map<std::string, bool>>();
				}
				scriptHandles_.push_back(sh);
			}
		}
	};
	std::string GetTypeName() const override { return "ScriptHandle"; }
};