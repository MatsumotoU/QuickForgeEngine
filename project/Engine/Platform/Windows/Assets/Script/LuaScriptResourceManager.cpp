#include "LuaScriptResourceManager.h"
#include <fstream>
#include <filesystem>

#include "Assets/AssetManager.h"
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
	maxPriority_ = 0;
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
	script->SetPriority(maxPriority_++);
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
	// 優先度を取得
	std::unordered_map<uint32_t, uint32_t> runSorts;
	for (auto& [handle, script] : scripts_) {
		runSorts[handle] = script->GetPriority();
	}
	// 優先度順にソート
	std::vector<std::pair<uint32_t, uint32_t>> sortedScripts(runSorts.begin(), runSorts.end());
	std::sort(sortedScripts.begin(), sortedScripts.end(), [](const auto& a, const auto& b) {
		return a.second < b.second;
		});
	for (const auto& [handle, priority] : sortedScripts) {
		auto script = GetScript(handle);
		// スクリプトが存在しない場合はスキップ
		if (!script) {
#ifdef _DEBUG
			DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
			continue;
		}
		// Lua状態が存在しない場合はスキップ
		if (script->HasFunction("Init")) {
			script->RunFunction("Init");
		}
	}
}

void LuaScriptResourceManager::InitializeScript(uint32_t handle) {
	if (scripts_.find(handle) == scripts_.end()) {
#ifdef _DEBUG
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
		return;
	}

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

void LuaScriptResourceManager::UpdateAllScripts() {
	// 優先度を取得
	std::unordered_map<uint32_t, uint32_t> runSorts;
	for (auto& [handle, script] : scripts_) {
		runSorts[handle] = script->GetPriority();
	}
	// 優先度順にソート
	std::vector<std::pair<uint32_t, uint32_t>> sortedScripts(runSorts.begin(), runSorts.end());
	std::sort(sortedScripts.begin(), sortedScripts.end(), [](const auto& a, const auto& b) {
		return a.second < b.second;
		});
	for (const auto& [handle, priority] : sortedScripts) {
		auto script = GetScript(handle);
		// スクリプトが存在しない場合はスキップ
		if (!script) {
#ifdef _DEBUG
			DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
			continue;
		}
		// Lua状態が存在しない場合はスキップ
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

sol::object LuaScriptResourceManager::GetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::state_view callScriptState) {
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
#ifdef _DEBUG
			DebugLog("Get Script Global: " + varName + " from " + scriptName, LogLevel::EngineInfo);
			DebugLog("Get Handle: " + std::to_string(handle.handle_), LogLevel::EngineInfo);
#endif // _DEBUG
			LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(handle.handle_);
			if (script) {
				sol::state* state = script->GetScript();
				sol::object obj = (*state)[varName];

				sol::state_view callerState = callScriptState;
				if (obj.is<sol::table>()) {
					sol::table srcTable = obj.as<sol::table>();
					sol::table dstTable = callerState.create_table();
					// 再帰的にコピー
					CopyLuaTable(srcTable, dstTable);
					return sol::make_object(callerState, dstTable);
				}
				return obj;
			}
		}
	}
#ifdef _DEBUG
	DebugLog("Script is not found", LogLevel::Warning);
#endif // _DEBUG
	return sol::nil;
}

void LuaScriptResourceManager::RunFunction(uint32_t entityId, const std::string& scriptName, const std::string& functionName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
#ifdef _DEBUG
		DebugLog("Entity has no ScriptHandles component", LogLevel::Warning);
#endif // _DEBUG
		return;
	}
	ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
	for (const auto& handle : scriptHandles.scriptHandles_) {
		if (handle.scriptName_ == scriptName) {
#ifdef _DEBUG
			DebugLog("Run Script Function: " + functionName + " from " + scriptName, LogLevel::EditorInfo);
			DebugLog("Run Handle: " + std::to_string(handle.handle_), LogLevel::EditorInfo);
#endif // _DEBUG
			LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(handle.handle_);
			if (script) {
				script->RunFunction(functionName);
			}
			return;
		}
	}
}

void LuaScriptResourceManager::CopyLuaTable(const sol::table& src, sol::table& dst) {
	for (auto& pair : src) {
		if (pair.second.is<sol::table>()) {
			sol::table newTable = sol::state_view(dst.lua_state()).create_table();
			CopyLuaTable(pair.second.as<sol::table>(), newTable);
			dst.set(pair.first, newTable);
		} else {
			dst.set(pair.first, pair.second);
		}
	}
}

void LuaScriptResourceManager::CheckScriptEntity() {
	for (auto& [handle, script] : scripts_) {
		// 最大優先度の更新
		if (maxPriority_ < script->GetPriority()) {
			maxPriority_ = script->GetPriority();
		}

		// エンティティが存在しない場合は削除予約
		if (!script) {
			continue;
		}
		if (!script->IsAliveEntity()) {
			removeScriptHandles_.push_back(handle);
		}
	}
}
