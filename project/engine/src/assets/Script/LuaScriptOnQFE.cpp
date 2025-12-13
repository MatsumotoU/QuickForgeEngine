#include "engine/include/assets/Script/LuaScriptOnQFE.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"

#include "engine/include/core/Math/Transform.h"
#include "engine/include/physics/Force.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

LuaScriptOnQFE::LuaScriptOnQFE() {
	isCanRun_ = false;
	scriptName_ = "";
	bindEntityId_ = 0;
	priority_ = 0;
	defaultGlobals.clear();
	UserGlobals.clear();
}

void LuaScriptOnQFE::LoadScript(const std::string& scriptName) {
	isCanRun_ = false;
	try {
		luaState_ = std::make_unique<sol::state>();
		luaState_->open_libraries(
			sol::lib::base,
			sol::lib::package,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table,
			sol::lib::coroutine,
			sol::lib::debug,
			sol::lib::utf8
		);

		// 襍ｷ蜍慕峩蠕後・繧ｰ繝ｭ繝ｼ繝舌Ν荳隕ｧ繧剃ｿ晏ｭ・
		for (auto& kv : luaState_->globals()) {
			defaultGlobals.insert(kv.first.as<std::string>());
		}

		std::string filePath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + scriptName;
		sol::load_result loadResult = luaState_->load_file(filePath);

		if (!loadResult.valid()) {
			sol::error err = loadResult;
			throw std::runtime_error("Failed to load Lua script: " + scriptName + "\n" + err.what());
		}

		SetQFEFunctions();

		// 繧ｹ繧ｯ繝ｪ繝励ヨ繧貞ｮ溯｡・
		sol::protected_function_result execResult = loadResult();
		if (!execResult.valid()) {
			sol::error err = execResult;
			throw std::runtime_error("Failed to execute Lua script: " + scriptName + "\n" + err.what());
		}

		// User繧ｰ繝ｭ繝ｼ繝舌Ν荳隕ｧ繧剃ｿ晏ｭ假ｼ医せ繧ｯ繝ｪ繝励ヨ螳溯｡悟ｾ鯉ｼ・ｼ・
		for (auto& kv : luaState_->globals()) {
			if (defaultGlobals.find(kv.first.as<std::string>()) == defaultGlobals.end()) {
				UserGlobals.insert(kv.first.as<std::string>());
			}
		}
		
		isCanRun_ = true;
		scriptName_ = scriptName;
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(e.what(), LogLevel::Error);
#else
		std::cerr << e.what() << std::endl;
#endif
		isCanRun_ = false;
	}
}

void LuaScriptOnQFE::ReloadScript() {
	if (scriptName_.empty()) {
		return;
	}

	std::set<std::string> oldGlobals = UserGlobals;
	LoadScript(scriptName_);
	// 蜿､縺・げ繝ｭ繝ｼ繝舌Ν螟画焚繧呈眠縺励＞繧ｹ繧ｯ繝ｪ繝励ヨ縺ｫ繧ｳ繝斐・
	for (const auto& global : oldGlobals) {
		if (UserGlobals.find(global) != UserGlobals.end()) {
			sol::object oldObj = luaState_->get<sol::object>(global);
			if (oldObj.is<int>()) {
				int v = oldObj.as<int>();
				luaState_->set(global, v);
			} else if (oldObj.is<float>()) {
				float v = oldObj.as<float>();
				luaState_->set(global, v);
			} else if (oldObj.is<bool>()) {
				bool v = oldObj.as<bool>();
				luaState_->set(global, v);
			} else if (oldObj.is<std::string>()) {
				std::string v = oldObj.as<std::string>();
				luaState_->set(global, v);
			}
		}
	}
}

bool LuaScriptOnQFE::HasFunction(const std::string& functionName) const {
	if (!luaState_ || !luaState_->lua_state()) {
#ifdef _DEBUG
		DebugLog("Lua state is not initialized.", LogLevel::Error);
#endif // _DEBUG
		return false;
	}
	sol::object obj = luaState_->get<sol::object>(functionName);
	return obj.is<sol::function>();
}

std::vector<std::string> LuaScriptOnQFE::GetFunctionList() const {
	std::vector<std::string> functionNames;
	if (luaState_) {
		sol::table globals = luaState_->globals();
		for (const auto& pair : globals) {
			if (pair.second.is<sol::function>()) {
				functionNames.push_back(pair.first.as<std::string>());
			}
		}
	}
	return functionNames;
}

sol::state* LuaScriptOnQFE::GetScript() const {
	if (!luaState_||!luaState_.get()) {
		return nullptr;
	}

	return luaState_.get();
}

const bool& LuaScriptOnQFE::IsCanRun() const {
	return isCanRun_;
}

bool LuaScriptOnQFE::IsAliveEntity() {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	if (!entityManager->IsActiveEntity(bindEntityId_)) {
		return false;
	}
	if (entityManager->HasComponent<ScriptHandles>(bindEntityId_)) {
		return true;
	}
	return false;
}

void LuaScriptOnQFE::SetEntityValue(uint32_t entityId) {
	bindEntityId_ = entityId;
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	// transform繧ｳ繝ｳ繝昴・繝阪Φ繝医ｒLua縺ｫ繧ｻ繝・ヨ
	try
	{
		if (entityManager->HasComponent<Transform>(bindEntityId_))
		{
			Transform& transform = entityManager->GetComponent<Transform>(bindEntityId_);
			luaState_->set("transform", &transform);
		} else
		{
			throw std::runtime_error("Entity does not have Transform component.");
		}
	}
	catch (const std::exception&)
	{
#ifdef _DEBUG
		DebugLog("Failed to set entity values in Lua script.", LogLevel::Error);
#endif // _DEBUG
	}

	// Force繧ｳ繝ｳ繝昴・繝阪Φ繝医ｒLua縺ｫ繧ｻ繝・ヨ
	if (entityManager->HasComponent<Force>(bindEntityId_))
	{
		Force& force = entityManager->GetComponent<Force>(bindEntityId_);
		luaState_->set("force", &force);
#ifdef _DEBUG
		DebugLog(std::format( "{}: Active ForceComponent.",scriptName_), LogLevel::EditorInfo);
#endif // _DEBUG
	}
}

std::vector<std::string> LuaScriptOnQFE::GetGlobalValuesList() const {
	std::vector<std::string> globalNames;
	for (const auto& global : UserGlobals) {
		sol::object obj = luaState_->get<sol::object>(global);
		if (obj.is<sol::function>()) {
			continue;
		}
		globalNames.push_back(global);
	}
	return globalNames;
}

void LuaScriptOnQFE::SetQFEFunctions() {
	// 繝・・繝悶Ν菴懈・
	sol::table qfe = luaState_->create_named_table("QFE");
	// QFE髢｢謨ｰ逋ｻ骭ｲ
	QFE::Script::SetQFEFunctions(luaState_.get());

	// this繧ｨ繝ｳ繝・ぅ繝・ぅ諠・ｱ逋ｻ骭ｲ
	sol::table thisEntity = luaState_->create_named_table("this");
	thisEntity.set_function("GetEntityId", [this]() {
		return bindEntityId_;
		});

	// 險ｳ繧｢繝ｪ髢｢謨ｰ鄒､
	// Log
	luaState_->set_function("DebugLog", [this](sol::variadic_args message) {
		message;
#ifdef _DEBUG
		DebugLogLua(message,this->GetBindEntityId(),this->GetScriptName());
#endif // _DEBUG
		});
	luaState_->set_function("GetThisEntityId", [this]() {
		return bindEntityId_;
		});
	luaState_->set_function("RunEntityScriptFunction",
		[](uint32_t entityId, const std::string& scriptName, const std::string& functionName) {
			LuaScriptResourceManager::GetInstance()->RunFunction(entityId, scriptName, functionName);
		}
	);
	luaState_->set_function("destroy", [this]() {
		AssetManager::GetInstance()->GetEntityManager()->RemoveEntity(this->GetBindEntityId());
		});
	luaState_->set_function("GetEntityScriptGlobal",
		[this](uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::this_state ts) {
			sol::state_view callerState(ts);
			return LuaScriptResourceManager::GetInstance()->GetEntityScriptGlobal(entityId, scriptName, varName, callerState);
		}
	);
}
