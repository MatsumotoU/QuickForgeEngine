#include "LuaScriptResourceManager.h"
#include "Assets/AssetManager.h"
#include <fstream>
#include <filesystem>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void LuaScriptResourceManager::Initialize() {
	scripts_.clear();
	removeScriptHandles_.clear();
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
		return ;
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
	scripts_.emplace_back();
	scripts_.back() = std::make_unique<LuaScriptOnQFE>();
	scripts_.back()->LoadScript(scriptName);
	scripts_.back()->SetEntityValue(entityId);

	return static_cast<uint32_t>(scripts_.size() - 1);
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
	if (handle < scripts_.size()) {
		scripts_.erase(scripts_.begin() + handle);
	}
}

void LuaScriptResourceManager::InitializeAllScripts() {
	for (auto& script : scripts_) {
		if (script->HasFunction("Init")) {
			script->RunFunction("Init");
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

void LuaScriptResourceManager::RunColliderStay(uint32_t aEintityId, uint32_t bEintityId) {
	if (scripts_.empty()) {
#ifdef _DEBUG
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // _DEBUG
		return;
	}

	for (auto& script : scripts_) {
		if (script->GetBindEntityId() != aEintityId && script->GetBindEntityId() != bEintityId) {
			continue;
		}

		if (script->HasFunction("OnCollisonStay")) {
			script->RunFunction("OnCollisonStay");
		} else {
#ifdef _DEBUG
			DebugLog("OnCollisonStay Not Found", LogLevel::Warning);
#endif // _DEBUG
		}
	}
}

void LuaScriptResourceManager::EndFrame() {
	CheckScriptEntity();
	for (uint32_t& handle : removeScriptHandles_) {
		RemoveScript(handle);
	}
	removeScriptHandles_.clear();
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
