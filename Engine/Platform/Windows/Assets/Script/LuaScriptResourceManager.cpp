#include "LuaScriptResourceManager.h"

void LuaScriptResourceManager::Initialize() {
	scripts_.clear();
	removeScriptHandles_.clear();
}

uint32_t LuaScriptResourceManager::AddScript(uint32_t entityId, const std::string& scriptName) {
	scripts_.emplace_back();
	scripts_.back() = std::make_unique<LuaScriptOnQFE>();
	scripts_.back()->LoadScript(scriptName);
	scripts_.back()->SetEntityValue(entityId);
	return static_cast<uint32_t>(scripts_.size() - 1);
}

void LuaScriptResourceManager::RemoveScript(uint32_t handle) {
	if (handle < scripts_.size()) {
		scripts_.erase(scripts_.begin() + handle);
	}
}

void LuaScriptResourceManager::InitializeAllScripts() {
	for (auto& script : scripts_) {
		if (script->HasFunction("Initialize")) {
			script->RunFunction("Initialize");
		}
	}
}

void LuaScriptResourceManager::UpdateAllScripts() {
	for (auto& script : scripts_) {
		if (script->HasFunction("Update")) {
			script->RunFunction("Update");
		}
	}
}

void LuaScriptResourceManager::EndFrame() {
	removeScriptHandles_.clear();
	CheckScriptEntity();
	for (uint32_t& handle : removeScriptHandles_) {
		RemoveScript(handle);
	}
}

void LuaScriptResourceManager::Finalize() {
	scripts_.clear();
}

void LuaScriptResourceManager::CheckScriptEntity() {
	uint32_t handle = 0;
	for (auto& script : scripts_) {
		if (!script->IsAliveEntity()) {
			removeScriptHandles_.push_back(handle);
		}
		handle++;
	}

}
