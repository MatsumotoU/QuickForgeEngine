#include "engine/include/assets/Script/LuaScriptOnQFE.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "Engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptExecutor.h"

#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/physics/Force.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"
#include "engine/include/collider/Data/AABBColliderData.h"
#include "engine/include/collider/Data/SphereColliderData.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

using namespace QFE;

LuaScriptOnQFE::LuaScriptOnQFE(EntityManager* entityManager, LuaScriptExecutor* luaScriptExecutor) {
	isCanRun_ = false;
	scriptName_ = "";
	bindEntityId_ = 0;
	priority_ = 0;
	defaultGlobals.clear();
	entityManager_ = entityManager;
	luaScriptExecutor_ = luaScriptExecutor;
	luaState_ = nullptr;
	handle_ = 0;
}

LuaScriptOnQFE::~LuaScriptOnQFE() {
	if (luaState_) {
		try {
			// Unregister update function
			(*luaState_)["QFE_Internal"]["UnregisterUpdate"](handle_);
		}
		catch (const sol::error& e) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::string("Lua Error in ~LuaScriptOnQFE: ") + e.what(), LogLevel::Error);
#endif
		}
	}
}

void QFE::LuaScriptOnQFE::Initialize(sol::state* state, const std::string& scriptPath, uint32_t bindId, uint32_t handle) {
	luaState_ = state;
	bindEntityId_ = bindId;
	handle_ = handle;
	// スクリプト名をパスから抽出
	size_t lastSlash = scriptPath.find_last_of("/\\");
	std::string scriptName = (lastSlash == std::string::npos) ? scriptPath : scriptPath.substr(lastSlash + 1);
	LoadScript(scriptName);
}

void LuaScriptOnQFE::LoadScript(const std::string& scriptName) {
	isCanRun_ = false;
	try {
		// 1. 小部屋に環境を作成
		sol::table env_table = luaState_->create_table();
		sol::table mt = luaState_->create_table();
		mt["__index"] = [this](sol::table t, sol::object key) -> sol::object {
			t;// TODO: 使っていない変数

			if (key.is<std::string>()) {
				std::string k = key.as<std::string>();
				auto* em = entityManager_;
				if (k == "transform") {
					if (em->HasComponent<Transform>(this->bindEntityId_)) {
						return sol::make_object(*luaState_, &em->GetComponent<Transform>(this->bindEntityId_));
					}
				} else if (k == "force") {
					if (em->HasComponent<Force>(this->bindEntityId_)) {
						return sol::make_object(*luaState_, &em->GetComponent<Force>(this->bindEntityId_));
					}
				} else if (k == "aabbCollider") {
					if (em->HasComponent<AABBColliderData>(this->bindEntityId_)) {
						return sol::make_object(*luaState_, &em->GetComponent<AABBColliderData>(this->bindEntityId_));
					}
				} else if (k == "sphereCollider") {
					if (em->HasComponent<SphereColliderData>(this->bindEntityId_)) {
						return sol::make_object(*luaState_, &em->GetComponent<SphereColliderData>(this->bindEntityId_));
					}
				}
			}
			return luaState_->globals()[key];
			};

		env_table[sol::metatable_key] = mt;

		// 2. 
		environment_ = sol::environment(luaState_->lua_state(), env_table);

		std::string filePath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + scriptName;
		sol::load_result loadResult = luaState_->load_file(filePath);

		if (!loadResult.valid()) {
			sol::error err = loadResult;
			throw std::runtime_error("Failed to load Lua script: " + scriptName + "\n" + err.what());
		}

		SetQFEFunctions();

		sol::function scriptFunc = loadResult;
		sol::set_environment(environment_, scriptFunc);

		sol::protected_function_result execResult = scriptFunc();
		if (!execResult.valid()) {
			sol::error err = execResult;
			throw std::runtime_error("Failed to execute Lua script: " + scriptName + "\n" + err.what());
		}

		isCanRun_ = true;
		scriptName_ = scriptName;


		initFunc_ = environment_["Init"];
		updateFunc_ = environment_["Update"];
		onCollisionEnterFunc_ = environment_["OnCollisionEnter"];
		onCollisionStayFunc_ = environment_["OnCollisionStay"];


		if (updateFunc_.valid()) {
			(*luaState_)["QFE_Internal"]["RegisterUpdate"](handle_, updateFunc_, priority_);
		}
	}

	catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
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

	LoadScript(scriptName_);
}


bool LuaScriptOnQFE::HasFunction(const std::string& functionName) const {
	sol::object obj = environment_[functionName];
	return obj.is<sol::function>();
}


std::vector<std::string> LuaScriptOnQFE::GetFunctionList() const {
	std::vector<std::string> functionNames;
	for (auto& kv : environment_) {
		if (kv.second.is<sol::function>()) {
			functionNames.push_back(kv.first.as<std::string>());
		}
	}
	return functionNames;
}


sol::state* LuaScriptOnQFE::GetScript() const {
	return luaState_;
}


void LuaScriptOnQFE::RunInit() {
	if (initFunc_.valid()) {
		initFunc_();
	}
}

void LuaScriptOnQFE::RunUpdate() {
	if (updateFunc_.valid()) {
		updateFunc_();
	}
}

void LuaScriptOnQFE::RunCollisionEnter(uint32_t id, SceneObjectData* objData) {
	if (onCollisionEnterFunc_.valid()) {
		onCollisionEnterFunc_(id, objData);
	}
}

void LuaScriptOnQFE::RunCollisionStay(uint32_t id, SceneObjectData* objData) {
	if (onCollisionStayFunc_.valid()) {
		onCollisionStayFunc_(id, objData);
	}
}

const bool& LuaScriptOnQFE::IsCanRun() const {
	return isCanRun_;
}

bool LuaScriptOnQFE::IsAliveEntity() {
	if (!entityManager_->IsActiveEntity(bindEntityId_)) {
		return false;
	}
	if (entityManager_->HasComponent<ScriptHandles>(bindEntityId_)) {
		return true;
	}
	return false;
}

void LuaScriptOnQFE::SetEntityValue(uint32_t entityId) {
	bindEntityId_ = entityId;
}



std::vector<std::string> LuaScriptOnQFE::GetGlobalValuesList() const {
	std::vector<std::string> globalNames;
	for (auto& kv : environment_) {
		sol::object obj = kv.second;
		if (obj.is<sol::function>()) {
			continue;
		}
		globalNames.push_back(kv.first.as<std::string>());
	}
	return globalNames;
}


void LuaScriptOnQFE::SetQFEFunctions() {
	// this繧ｨ繝ｳ繝・ぅ繝・ぅ諠・ｱ逋ｻ骭ｲ
	sol::table thisEntity = luaState_->create_table();
	thisEntity.set_function("GetEntityId", [this]() {
		return bindEntityId_;
		});
	environment_["this"] = thisEntity;

	// 險ｳ繧｢繝ｪ髢｢謨ｰ鄒､
	// Log
	environment_["DebugLog"] = [this](sol::variadic_args message) {
		message;
#ifdef QFE_OPTIMIZE_OFF
		DebugLogLua(message, this->GetBindEntityId(), this->GetScriptName());
#endif // QFE_OPTIMIZE_OFF
		};
	environment_["GetThisEntityId"] = [this]() {
		return bindEntityId_;
		};
	environment_["RunEntityScriptFunction"] =
		[this](uint32_t entityId, const std::string& scriptName, const std::string& functionName) {
			this->luaScriptExecutor_->RunFunction(entityId, scriptName, functionName);
		};
	environment_["destroy"] = [this]() {
		entityManager_->RemoveEntity(this->GetBindEntityId());
		};
	environment_["delete"] = [this]() {
		entityManager_->RemoveEntity(this->GetBindEntityId());
		};
	environment_["GetEntityScriptGlobal"] =
		[this](uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::this_state ts) {
			sol::state_view callerState(ts);
			return this->luaScriptExecutor_->GetEntityScriptGlobal(entityId, scriptName, varName, callerState);
		};
	environment_["SetEntityScriptGlobal"] =
		[this](uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::object value) {
			this->luaScriptExecutor_->SetEntityScriptGlobal(entityId, scriptName, varName, value);
		};

	environment_["SetAABBColiderSize"] = [this](const Vector3& size) {
		if (entityManager_->HasComponent<AABBColliderData>(this->GetBindEntityId()) == false) {
			return;
		}
		AABBColliderData& collider = entityManager_->GetComponent<AABBColliderData>(this->GetBindEntityId());
		collider.aabb.size = size;
		};
}
