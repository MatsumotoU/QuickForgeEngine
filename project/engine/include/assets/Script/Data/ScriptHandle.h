#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"
#include <vector>
#include <unordered_map>
namespace QFE {
	// Luaスクリプトのハンドル情報
	struct LuaHandle {
		std::string scriptName_;
		uint32_t priority_ = 0;
		std::unordered_map<std::string, int> intParams_;
		std::unordered_map<std::string, float> floatParams_;
		std::unordered_map<std::string, bool> boolParams_;
		std::unordered_map<std::string, std::string> stringParams_;
		uint32_t handle_ = 0;

		// コピー・ムーブの明示的な指定のためのデフォルトコンストラクタ
		LuaHandle() = default;
		// ムーブコンストラクタをnoexceptで明示
		LuaHandle(LuaHandle&&) noexcept = default;
		LuaHandle& operator=(LuaHandle&&) noexcept = default;
		// コピーコンストラクタ・代入演算子も必要なら明示
		LuaHandle(const LuaHandle&) = default;
		LuaHandle& operator=(const LuaHandle&) = default;
	};

	class ScriptHandles final :public ComponentData {
	public:
		std::vector<LuaHandle> scriptHandles_;

		ScriptHandles() = default;
		~ScriptHandles() override = default;
		nlohmann::json Serialize() const override {
			nlohmann::json json;
			for (const LuaHandle& scriptHandle : scriptHandles_) {
				nlohmann::json shJson;
				shJson["scriptName"] = scriptHandle.scriptName_;
				shJson["intParams"] = scriptHandle.intParams_;
				shJson["floatParams"] = scriptHandle.floatParams_;
				shJson["boolParams"] = scriptHandle.boolParams_;
				shJson["stringParams"] = scriptHandle.stringParams_;
				shJson["priority"] = scriptHandle.priority_;
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
					if (shJson.contains("intParams") && shJson["intParams"].is_object()) {
						sh.intParams_ = shJson["intParams"].get<std::unordered_map<std::string, int>>();
					}
					if (shJson.contains("floatParams") && shJson["floatParams"].is_object()) {
						sh.floatParams_ = shJson["floatParams"].get<std::unordered_map<std::string, float>>();
					}
					if (shJson.contains("boolParams") && shJson["boolParams"].is_object()) {
						sh.boolParams_ = shJson["boolParams"].get<std::unordered_map<std::string, bool>>();
					}
					if (shJson.contains("stringParams") && shJson["stringParams"].is_object()) {
						sh.stringParams_ = shJson["stringParams"].get<std::unordered_map<std::string, std::string>>();
					}
					if (shJson.contains("priority") && shJson["priority"].is_number_unsigned()) {
						sh.priority_ = shJson["priority"].get<uint32_t>();
					}
					scriptHandles_.push_back(sh);
				}
			}
		}
		std::string GetTypeName() const override { return "ScriptHandle"; }
	};
}