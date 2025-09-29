#pragma once
#include "Core/Entity/Component/ComponentData.h"
#include <vector>

struct LuaHandle {
	std::string scriptName_;
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
				scriptHandles_.push_back(sh);
			}
		}
	};
	std::string GetTypeName() const override { return "ScriptHandle"; }
};