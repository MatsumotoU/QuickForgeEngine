#include "engine/include/assets/Script/LuaScriptExecutor.h"
#include "engine/include/assets/Script/LuaRuntimeManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif

using namespace QFE;

LuaScriptExecutor::LuaScriptExecutor() : isRunningScript_(false), nextScriptHandle_(0) {}

void LuaScriptExecutor::Initialize(EntityManager* entityManager) {
	entityManager_ = entityManager;

	// LuaRuntimeManagerを使って新しいLua stateを作成
	sharedLuaState_ = LuaRuntimeManager::GetInstance()->CreateLuaState(entityManager, this);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("LuaScriptExecutor initialized with new Lua state.");
#endif
}

void LuaScriptExecutor::FrameStart() {
	isRunningScript_ = true;
}

void LuaScriptExecutor::Reset() {
	scripts_.clear();
	nextScriptHandle_ = 0;

	// Lua側の更新リストもクリア
	if (sharedLuaState_) {
		sharedLuaState_->script("QFE_Internal.ClearList()");
	}

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("LuaScriptExecutor reset.");
#endif
}

void QFE::LuaScriptExecutor::ReloadAllScripts() {
	// すべてのスクリプトをリロード
	for (auto& [handle, script] : scripts_) {
		if (script) {
			script->ReloadScript();
		}
	}
}

uint32_t LuaScriptExecutor::AddScript(uint32_t entityId, const std::string& scriptName) {
	if (!sharedLuaState_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Lua state not initialized. Cannot add script.", LogLevel::Error);
#endif
		return 0;
	}

	// スクリプトファイルのパスを取得
	std::string scriptDir = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	std::string scriptPath = scriptDir + scriptName;

	uint32_t handle = nextScriptHandle_++;

	// スクリプトを作成
	auto script = std::make_unique<LuaScriptOnQFE>(entityManager_, this);
	script->Initialize(sharedLuaState_.get(), scriptPath, entityId, handle);

	scripts_[handle] = std::move(script);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Added Lua script: " + scriptName + " (handle: " + std::to_string(handle) + ")");
#endif

	return handle;
}

void LuaScriptExecutor::RemoveScript(uint32_t handle) {
	auto it = scripts_.find(handle);
	if (it != scripts_.end()) {
		scripts_.erase(it);
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Removed Lua script (handle: " + std::to_string(handle) + ")");
#endif
	}
	else {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Script handle not found: " + std::to_string(handle), LogLevel::Warning);
#endif
	}
}

void LuaScriptExecutor::InitializeAllScripts() {
	for (auto& [handle, script] : scripts_) {
		if (script) {
			script->RunFunction("Init");
		}
	}
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Initialized all Lua scripts.");
#endif
}

void LuaScriptExecutor::UpdateAllScripts() {
	if (!sharedLuaState_) {
		return;
	}

	// Lua側の更新システムを呼び出し
	try {
		sharedLuaState_->script("QFE_Internal.UpdateAll()");
	}
	catch (const sol::error& e) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::string("Lua Update Error: ") + e.what(), LogLevel::Error);
#endif
	}
}

void LuaScriptExecutor::RemoveDeadScripts() {
	for (auto it = scripts_.begin(); it != scripts_.end();) {
		if (it->second && !it->second->IsAliveEntity()) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Removing dead script: " + it->second->GetScriptName() + " (handle: " + std::to_string(it->first) + ")");
#endif
			it = scripts_.erase(it);
		}
		else {
			++it;
		}
	}
}

void LuaScriptExecutor::RunAllFunction(const std::string& functionName) {
	for (auto& [handle, script] : scripts_) {
		if (script) {
			script->RunFunction(functionName);
		}
	}
}

void QFE::LuaScriptExecutor::RunEntityFunction(uint32_t entityId, const std::string& functionName) {
	for (auto& [handle, script] : scripts_) {
		if (script && script->GetBindEntityId() == entityId) {
			script->RunFunction(functionName);
		}
	}
}

void QFE::LuaScriptExecutor::RunFunctionIfExists(const std::string& functionName) {
	for (auto& [handle, script] : scripts_) {
		if (script) {
			sol::environment& env = script->GetEnvironment();
			sol::object obj = env[functionName];
			if (obj.is<sol::function>()) {
				script->RunFunction(functionName);
			}
		}
	}
}

void QFE::LuaScriptExecutor::RunEntityFunctionIfExists(uint32_t entityId, const std::string& functionName) {
	for (auto& [handle, script] : scripts_) {
		if (script && script->GetBindEntityId() == entityId) {
			sol::environment& env = script->GetEnvironment();
			sol::object obj = env[functionName];
			if (obj.is<sol::function>()) {
				script->RunFunction(functionName);
			}
		}
	}
}

void QFE::LuaScriptExecutor::RunFunction(uint32_t entityId, const std::string& scriptName, const std::string& functionName) {
	for (auto& [handle, script] : scripts_) {
		if (script && script->GetBindEntityId() == entityId && script->GetScriptName() == scriptName) {
			script->RunFunction(functionName);
		}
	}
}

sol::object LuaScriptExecutor::GetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::state_view& callerState) {
	for (auto& [handle, script] : scripts_) {
		if (script && script->GetBindEntityId() == entityId && script->GetScriptName() == scriptName) {
			sol::environment& env = script->GetEnvironment();
			sol::object val = env[varName];
			if (val.valid()) {
				return sol::object(callerState, val);
			}
		}
	}
	return sol::object(sol::nil);
}

void LuaScriptExecutor::SetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName, const sol::object& value) {
	for (auto& [handle, script] : scripts_) {
		if (script && script->GetBindEntityId() == entityId && script->GetScriptName() == scriptName) {
			sol::environment& env = script->GetEnvironment();
			env[varName] = value;
		}
	}
}

LuaScriptOnQFE* LuaScriptExecutor::GetScript(uint32_t handle) const {
	auto it = scripts_.find(handle);
	if (it != scripts_.end()) {
		return it->second.get();
	}
	return nullptr;
}
