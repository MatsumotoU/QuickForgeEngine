#include "engine/include/assets/Script/LuaScriptResourceManager.h"
#include <fstream>
#include <filesystem>

#include "engine/include/Assets/AssetManager.h"
#include "engine/include/Core/Entity/EntityManager.h"

#include "engine/include/Assets/Script/Data/ScriptHandle.h"
#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"


#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF
static std::set<std::string> emptySet;
using namespace QFE;

LuaScriptResourceManager::LuaScriptResourceManager() :
	isRunningScript_(false),
	maxPriority_(0),
	nextScriptHandle_(0) {
}
void LuaScriptResourceManager::Initialize() {
	sharedLuaState_ = std::make_unique<sol::state>();
	sharedLuaState_->open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::math,
		sol::lib::string,
		sol::lib::table,
		sol::lib::coroutine,
		sol::lib::debug,
		sol::lib::utf8
	);

	// QFE縺ｨ縺・≧蜷榊燕縺ｮ繧ｰ繝ｭ繝ｼ繝舌Ν繝・・繝悶Ν繧剃ｽ懈・
	sharedLuaState_->create_named_table("QFE");

	Script::SetQFEFunctions(sharedLuaState_.get());

	// Lua蛛ｴ縺ｮ繝ｬ繧ｸ繧ｹ繝医Μ繧剃ｽ懈・
	sharedLuaState_->script(R"(
		QFE_Internal = {
			update_list = {},
			UpdateAll = function()
				if QFE_Internal.dirty then
					QFE_Internal.sorted_list = {}
					for handle, entry in pairs(QFE_Internal.update_list) do
						table.insert(QFE_Internal.sorted_list, entry)
					end
					table.sort(QFE_Internal.sorted_list, function(a, b) return a.priority < b.priority end)
					QFE_Internal.dirty = false
				end
				for _, entry in ipairs(QFE_Internal.sorted_list) do
					entry.func()
				end
			end,

			RegisterUpdate = function(handle, func, priority)
				QFE_Internal.update_list[handle] = {func = func, priority = priority}
				QFE_Internal.dirty = true
			end,
			UnregisterUpdate = function(handle)
				if QFE_Internal.update_list[handle] then
					QFE_Internal.update_list[handle] = nil
					QFE_Internal.dirty = true
				end
			end,
			ClearList = function()
				QFE_Internal.update_list = {}
				QFE_Internal.sorted_list = {}
				QFE_Internal.dirty = false
			end,
			dirty = false,
			sorted_list = {}
		}
	)");


	scripts_.clear();

	removeScriptHandles_.clear();
	isRunningScript_ = false;
	nextScriptHandle_ = 0;
	maxPriority_ = 0;

#ifdef QFE_OPTIMIZE_OFF
	viewUpdateTime_ = std::chrono::duration<double>::zero();
	totalUpdateTime_ = std::chrono::duration<double>::zero();
#endif // QFE_OPTIMIZE_OFF
}

void LuaScriptResourceManager::FrameStart() {
#ifdef QFE_OPTIMIZE_OFF
	totalUpdateTime_ = std::chrono::duration<double>::zero();
#endif // QFE_OPTIMIZE_OFF
}


void LuaScriptResourceManager::Reset() {
	if (sharedLuaState_) {
		(*sharedLuaState_)["QFE_Internal"]["ClearList"]();
	}
	scripts_.clear();
	removeScriptHandles_.clear();
	nextScriptHandle_ = 0;
	maxPriority_ = 0;
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

void LuaScriptResourceManager::RunAllFunction(const std::string& functionName) {
#ifdef QFE_OPTIMIZE_OFF
	auto startTime = std::chrono::steady_clock::now();
#endif // QFE_OPTIMIZE_OFF
	for (auto& [handle, script] : scripts_) {
		if (script && script->HasFunction(functionName)) {
			script->RunFunction(functionName);
		}
	}
#ifdef QFE_OPTIMIZE_OFF
	auto endTime = std::chrono::steady_clock::now();
	totalUpdateTime_ += endTime - startTime;
#endif // QFE_OPTIMIZE_OFF
}

void LuaScriptResourceManager::CreateScript(const std::string& scriptName) {
	// 繝・ぅ繝ｬ繧ｯ繝医Μ繝代せ
	const std::string dirPath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	// 繝・ぅ繝ｬ繧ｯ繝医Μ縺後↑縺代ｌ縺ｰ菴懈・
	std::filesystem::create_directories(dirPath);

	std::string loadScriptName = scriptName;

	// 諡｡蠑ｵ蟄舌′縺､縺・※縺・ｋ縺狗｢ｺ隱・
	if (!loadScriptName.ends_with(".lua")) {
		loadScriptName += ".lua";
	}

	// 繝輔ぃ繧､繝ｫ繝代せ
	std::string filePath = dirPath + scriptName;

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Create Lua Script: " + filePath, LogLevel::EditorInfo);
#endif // QFE_OPTIMIZE_OFF

	// Lua繝・Φ繝励Ξ繝ｼ繝・
	const char* luaTemplate =
		"function Init()\n"
		"\n"
		"end\n"
		"\n"
		"function Update()\n"
		"\n"
		"end\n";

	// 繝輔ぃ繧､繝ｫ譖ｸ縺崎ｾｼ縺ｿ
	std::ofstream ofs(filePath);
	if (!ofs) {
		return;
	}
	ofs << luaTemplate;
	ofs.close();

	// 閾ｪ蜍輔〒髢九￥
	try {
		std::filesystem::path absPath = std::filesystem::absolute(filePath);
		ShellExecuteA(nullptr, "open", "code", absPath.string().c_str(), nullptr, SW_SHOWNORMAL);
	}
	catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(e.what(), LogLevel::Error);
#else
		std::cerr << e.what() << std::endl;
#endif
	}
}

uint32_t LuaScriptResourceManager::AddScript(uint32_t entityId, const std::string& scriptName) {
	uint32_t handle = nextScriptHandle_++;
	auto script = std::make_unique<LuaScriptOnQFE>();
	script->SetHandle(handle);
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
	// 繝輔ぃ繧､繝ｫ繝代せ
	std::string filePath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + scriptName;
	try {
		std::filesystem::path absPath = std::filesystem::absolute(filePath);
		ShellExecuteA(nullptr, "open", "code", absPath.string().c_str(), nullptr, SW_SHOWNORMAL);
	}
	catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(e.what(), LogLevel::Error);
#else
		std::cerr << e.what() << std::endl;
#endif
	}
}

void LuaScriptResourceManager::RemoveScript(uint32_t handle) {
	if (sharedLuaState_) {
		(*sharedLuaState_)["QFE_Internal"]["UnregisterUpdate"](handle);
	}
	scripts_.erase(handle);
}


void LuaScriptResourceManager::InitializeAllScripts() {
	// 蜆ｪ蜈亥ｺｦ繧貞叙蠕・
	std::unordered_map<uint32_t, uint32_t> runSorts;
	for (auto& [handle, script] : scripts_) {
		runSorts[handle] = script->GetPriority();
	}
	// 蜆ｪ蜈亥ｺｦ鬆・↓繧ｽ繝ｼ繝・
	std::vector<std::pair<uint32_t, uint32_t>> sortedScripts(runSorts.begin(), runSorts.end());
	std::sort(sortedScripts.begin(), sortedScripts.end(), [](const auto& a, const auto& b) {
		return a.second < b.second;
		});
	for (const auto& [handle, priority] : sortedScripts) {
		auto script = GetScript(handle);
		// 繧ｹ繧ｯ繝ｪ繝励ヨ縺悟ｭ伜惠縺励↑縺・ｴ蜷医・繧ｹ繧ｭ繝・・
		if (!script) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Script Not Found", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
			continue;
		}
		// Lua迥ｶ諷九′蟄伜惠縺励↑縺・ｴ蜷医・繧ｹ繧ｭ繝・・
		if (script->IsCanRun()) {
			script->RunInit();
		}

	}
}

void LuaScriptResourceManager::InitializeScript(uint32_t handle) {
#ifdef QFE_OPTIMIZE_OFF
	auto startTime = std::chrono::steady_clock::now();
#endif // QFE_OPTIMIZE_OFF

	if (scripts_.find(handle) == scripts_.end()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}

	auto& script = scripts_[handle];

	if (!script || !script->GetScript()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	if (script->IsCanRun()) {
		script->RunInit();
	}

#ifdef QFE_OPTIMIZE_OFF
	auto endTime = std::chrono::steady_clock::now();
	totalUpdateTime_ += endTime - startTime;
#endif // QFE_OPTIMIZE_OFF
}

void LuaScriptResourceManager::UpdateAllScripts() {
	if (!isRunningScript_) return;
#ifdef QFE_OPTIMIZE_OFF
	auto startTime = std::chrono::steady_clock::now();
#endif // QFE_OPTIMIZE_OFF

	try {
		sol::protected_function updateAll = (*sharedLuaState_)["QFE_Internal"]["UpdateAll"];
		auto result = updateAll();
		if (!result.valid()) {
			sol::error err = result;
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Lua UpdateAll error: " + std::string(err.what()), LogLevel::Error);
#endif
		}
	}
	catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Exception in UpdateAllAllScripts: " + std::string(e.what()), LogLevel::Error);
#endif
	}

#ifdef QFE_OPTIMIZE_OFF
	auto endTime = std::chrono::steady_clock::now();
	totalUpdateTime_ += endTime - startTime;
#endif // QFE_OPTIMIZE_OFF
}


void LuaScriptResourceManager::RunColliderStay(uint32_t runId, uint32_t id, SceneObjectData* objData) {
#ifdef QFE_OPTIMIZE_OFF
	auto startTime = std::chrono::steady_clock::now();
#endif // QFE_OPTIMIZE_OFF

	if (scripts_.empty()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}

	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(runId))
	{
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(runId);
		for (const auto& handle : scriptHandles.scriptHandles_) {
			auto script = GetScript(handle.handle_);
			if (script) {
				script->RunCollisionStay(id, objData);
			}

		}
		return;
	}

#ifdef QFE_OPTIMIZE_OFF
	auto endTime = std::chrono::steady_clock::now();
	totalUpdateTime_ += endTime - startTime;
#endif // QFE_OPTIMIZE_OFF
}

void LuaScriptResourceManager::RunTriggerEnter(uint32_t runId, uint32_t id, SceneObjectData* objData) {
#ifdef QFE_OPTIMIZE_OFF
	auto startTime = std::chrono::steady_clock::now();
#endif // QFE_OPTIMIZE_OFF

	if (scripts_.empty()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Script Not Found", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}

	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(runId))
	{
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(runId);
		for (const auto& handle : scriptHandles.scriptHandles_) {
			auto script = GetScript(handle.handle_);
			if (script) {
				script->RunCollisionEnter(id, objData);
			}

		}
		return;
	}

#ifdef QFE_OPTIMIZE_OFF
	auto endTime = std::chrono::steady_clock::now();
	totalUpdateTime_ += endTime - startTime;
#endif // QFE_OPTIMIZE_OFF
}

void LuaScriptResourceManager::EndFrame() {
	CheckScriptEntity();
	// 髯埼・〒蜑企勁
	std::sort(removeScriptHandles_.rbegin(), removeScriptHandles_.rend());
	for (uint32_t handle : removeScriptHandles_) {
		RemoveScript(handle);
	}
	removeScriptHandles_.clear();

	viewUpdateTime_ = totalUpdateTime_;
}

void LuaScriptResourceManager::Finalize() {
	scripts_.clear();
	sharedLuaState_.reset();
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
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Entity has no ScriptHandles component", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return sol::nil;
	}

	ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
	for (const auto& handle : scriptHandles.scriptHandles_) {
		if (handle.scriptName_ == scriptName) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Get Script Global: " + varName + " from " + scriptName, LogLevel::EngineInfo);
			DebugLog("Get Handle: " + std::to_string(handle.handle_), LogLevel::EngineInfo);
#endif // QFE_OPTIMIZE_OFF
			LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(handle.handle_);
			if (script) {
				sol::object obj = script->GetEnvironment()[varName];

				sol::state_view callerState = callScriptState;
				if (obj.is<sol::table>()) {
					sol::table srcTable = obj.as<sol::table>();
					sol::table dstTable = callerState.create_table();
					// 蜀榊ｸｰ逧・↓繧ｳ繝斐・
					CopyLuaTable(srcTable, dstTable);
					return sol::make_object(callerState, dstTable);
				}
				return obj;
			}

		}
	}
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Script is not found", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
	return sol::nil;
}

void LuaScriptResourceManager::SetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::object value) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Entity has no ScriptHandles component", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
	for (const auto& handle : scriptHandles.scriptHandles_) {
		if (handle.scriptName_ == scriptName) {
			LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(handle.handle_);
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Set Script Global: " + varName + " from " + scriptName, LogLevel::EngineInfo);
			DebugLog("Set Handle: " + std::to_string(handle.handle_), LogLevel::EngineInfo);
#endif // QFE_OPTIMIZE_OFF
			if (script) {
				script->GetEnvironment()[varName] = value;
			}

			return;
		}
	}
}

void LuaScriptResourceManager::RunFunction(uint32_t entityId, const std::string& scriptName, const std::string& functionName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Entity has no ScriptHandles component", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
	for (const auto& handle : scriptHandles.scriptHandles_) {
		if (handle.scriptName_ == scriptName) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Run Script Function: " + functionName + " from " + scriptName, LogLevel::EditorInfo);
			DebugLog("Run Handle: " + std::to_string(handle.handle_), LogLevel::EditorInfo);
#endif // QFE_OPTIMIZE_OFF
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
		// 譛€螟ｧ蜆ｪ蜈亥ｺｦ縺ｮ譖ｴ譁ｰ
		if (maxPriority_ < script->GetPriority()) {
			maxPriority_ = script->GetPriority();
		}

		// 繧ｨ繝ｳ繝・ぅ繝・ぅ縺悟ｭ伜惠縺励↑縺・ｴ蜷医・蜑企勁莠育ｴ・
		if (!script) {
			continue;
		}
		if (!script->IsAliveEntity()) {
			removeScriptHandles_.push_back(handle);
		}
	}
}
