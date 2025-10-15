#include "LuaScriptResourceManager.h"
#include "Assets/AssetManager.h"
#include <fstream>
#include <filesystem>

#include "Core/Entity/EntityManager.h"

#include "Assets/Script/Data/ScriptHandle.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
static std::set<std::string> emptySet;

void LuaScriptResourceManager::Initialize() {
	scripts_.clear();
	removeScriptHandles_.clear();
	isRunningScript_ = false;
	nextScriptHandle_ = 0;
}

void LuaScriptResourceManager::Reset() {
	scripts_.clear();
	removeScriptHandles_.clear();
}

void LuaScriptResourceManager::ReloadAllScripts() {
	if (isRunningScript_) {
		return;
	}

	for (auto& [handle, script] : scripts_) {
		if (script) {
			script->ReloadScript();
		}
	}
}

void LuaScriptResourceManager::CreateScript(const std::string& scriptName) {
	// ディレクトリパス
	const std::string dirPath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	// ディレクトリがなければ作成
	std::filesystem::create_directories(dirPath);

	std::string loadScriptName = scriptName;

	// 拡張子がついているか確認
	if (!loadScriptName.ends_with(".lua")) {
		loadScriptName += ".lua";
	}

	// ファイルパス
	std::string filePath = dirPath + scriptName;

#ifdef _DEBUG
	DebugLog("Create Lua Script: " + filePath, LogLevel::EditorInfo);
#endif // _DEBUG

	// Luaテンプレート
	const char* luaTemplate =
		"function Init()\n"
		"\n"
		"end\n"
		"\n"
		"function Update()\n"
		"\n"
		"end\n";

	// ファイル書き込み
	std::ofstream ofs(filePath);
	if (!ofs) {
		return;
	}
	ofs << luaTemplate;
	ofs.close();

	// 自動で開く
	try {
		std::filesystem::path absPath = std::filesystem::absolute(filePath);
		ShellExecuteA(nullptr, "open", "code", absPath.string().c_str(), nullptr, SW_SHOWNORMAL);
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(e.what(), LogLevel::Error);
#else
		std::cerr << e.what() << std::endl;
#endif
	}
}

uint32_t LuaScriptResourceManager::AddScript(uint32_t entityId, const std::string& scriptName) {
	uint32_t handle = nextScriptHandle_++;
	auto script = std::make_unique<LuaScriptOnQFE>();
	script->LoadScript(scriptName);
	script->SetEntityValue(entityId);
	scripts_.emplace(handle, std::move(script));
	return handle;
}

void LuaScriptResourceManager::RequestRemoveScript(uint32_t handle) {
	removeScriptHandles_.push_back(handle);
}

void LuaScriptResourceManager::OpenAndEditScript(const std::string& scriptName) {
	// ファイルパス
	std::string filePath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + scriptName;
	try {
		std::filesystem::path absPath = std::filesystem::absolute(filePath);
		ShellExecuteA(nullptr, "open", "code", absPath.string().c_str(), nullptr, SW_SHOWNORMAL);
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(e.what(), LogLevel::Error);
#else
		std::cerr << e.what() << std::endl;
#endif
	}
}

void LuaScriptResourceManager::RemoveScript(uint32_t handle) {
	scripts_.erase(handle);
}

void LuaScriptResourceManager::InitializeAllScripts() {
	for (auto& [handle, script] : scripts_) {
		if (!script) {
#ifdef _DEBUG
			DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
			continue;
		}

		if (script->HasFunction("Init")) {
			script->RunFunction("Init");
		}
	}
}

void LuaScriptResourceManager::InitializeScript(uint32_t handle) {
	if (handle < scripts_.size()) {
		auto& script = scripts_[handle];
		if (!script || !script->GetScript()) {
#ifdef _DEBUG
			DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
			return;
		}
		if (script->HasFunction("Init")) {
			script->RunFunction("Init");
		}
	}
}

void LuaScriptResourceManager::UpdateAllScripts() {
	for (auto& [handle, script] : scripts_) {
		if (!script) {
#ifdef _DEBUG
			DebugLog("Script pointer is nullptr", LogLevel::Warning);
#endif
			continue;
		}
		auto* state = script->GetScript();
		if (!state) {
#ifdef _DEBUG
			DebugLog("Lua state is nullptr", LogLevel::Warning);
#endif
			continue;
		}
		if (script->HasFunction("Update")) {
			script->RunFunction("Update");
		}
	}
}

void LuaScriptResourceManager::RunColliderStay(uint32_t runId, uint32_t id, SceneObjectData* objData) {
	if (scripts_.empty()) {
#ifdef _DEBUG
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
		return;
	}

	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(runId))
	{
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(runId);
		for (const auto& handle : scriptHandles.scriptHandles_) {
			auto script = GetScript(handle.handle_);
			if (script && script->HasFunction("OnCollisionStay")) {
				script->RunFunction("OnCollisionStay", id, objData);
			}
		}
		return;
	}
}

void LuaScriptResourceManager::RunTriggerEnter(uint32_t runId, uint32_t id, SceneObjectData* objData) {
	if (scripts_.empty()) {
#ifdef _DEBUG
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
		return;
	}

	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(runId))
	{
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(runId);
		for (const auto& handle : scriptHandles.scriptHandles_) {
			auto script = GetScript(handle.handle_);
			if (script && script->HasFunction("OnCollisionEnter")) {
				script->RunFunction("OnCollisionEnter", id, objData);
			}
		}
		return;
	}
}

void LuaScriptResourceManager::EndFrame() {
	CheckScriptEntity();
	// 降順で削除
	std::sort(removeScriptHandles_.rbegin(), removeScriptHandles_.rend());
	for (uint32_t handle : removeScriptHandles_) {
		RemoveScript(handle);
	}
	removeScriptHandles_.clear();
}

void LuaScriptResourceManager::Finalize() {
	scripts_.clear();
}

LuaScriptOnQFE* LuaScriptResourceManager::GetScript(uint32_t handle) const {
	auto it = scripts_.find(handle);
	if (it != scripts_.end()) {
		return it->second.get();
	}
	return nullptr;
}

std::set<std::string>& LuaScriptResourceManager::GetScriptGlobals(uint32_t entityId) const {
	for (auto& [handle, script] : scripts_) {
		if (script->GetBindEntityId() != entityId) {
			continue;
		}
		return script->GetGlobals();
	}
	assert(false && "Script Not Found");
	return emptySet;
}

sol::object LuaScriptResourceManager::GetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
#ifdef _DEBUG
		DebugLog("Entity has no ScriptHandles component", LogLevel::Warning);
#endif // _DEBUG
		return sol::nil;
	} 

	ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
	for (const auto& handle : scriptHandles.scriptHandles_) {
		if (handle.scriptName_ == scriptName) {
			LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(handle.handle_);
			if (script) {
				sol::state* state = script->GetScript();
				return (*state)[varName];
			}
		}
	}
#ifdef _DEBUG
	DebugLog("Script is not found", LogLevel::Warning);
#endif // _DEBUG
	return sol::nil;
}

void LuaScriptResourceManager::CheckScriptEntity() {
	for (auto& [handle, script] : scripts_) {
		if (!script) {
			continue;
		}

		if (!script->IsAliveEntity()) {
			removeScriptHandles_.push_back(handle);
		}
	}
}
