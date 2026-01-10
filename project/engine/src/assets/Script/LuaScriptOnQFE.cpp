#include "engine/include/assets/Script/LuaScriptOnQFE.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "Engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"

#include "engine/include/core/Math/Transform.h"
#include "engine/include/physics/Force.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"
#include "engine/include/collider/Data/AABBColliderData.h"
#include "engine/include/collider/Data/SphereColliderData.h"

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

LuaScriptOnQFE::LuaScriptOnQFE() {
	isCanRun_ = false;
	scriptName_ = "";
	bindEntityId_ = 0;
	priority_ = 0;
	defaultGlobals.clear();
	// defaultGlobals.clear(); // No longer needed with environments
	// UserGlobals.clear(); // No longer needed with environments
}

void LuaScriptOnQFE::LoadScript(const std::string& scriptName) {
	isCanRun_ = false;
	try {
		sol::state& luaState = LuaScriptResourceManager::GetInstance()->GetSharedState();
		
		// 1. 環境用テーブルを作成し、メタテーブルをセット
		sol::table env_table = luaState.create_table();
		sol::table mt = luaState.create_table();
		mt["__index"] = [this, &luaState](sol::table t, sol::object key) -> sol::object {
			if (key.is<std::string>()) {
				std::string k = key.as<std::string>();
				auto* em = AssetManager::GetInstance()->GetEntityManager();
				if (k == "transform") {
					if (em->HasComponent<Transform>(this->bindEntityId_)) {
						return sol::make_object(luaState, &em->GetComponent<Transform>(this->bindEntityId_));
					}
				}
				else if (k == "force") {
					if (em->HasComponent<Force>(this->bindEntityId_)) {
						return sol::make_object(luaState, &em->GetComponent<Force>(this->bindEntityId_));
					}
				}
				else if (k == "aabbCollider") {
					if (em->HasComponent<AABBColliderData>(this->bindEntityId_)) {
						return sol::make_object(luaState, &em->GetComponent<AABBColliderData>(this->bindEntityId_));
					}
				}
				else if (k == "sphereCollider") {
					if (em->HasComponent<SphereColliderData>(this->bindEntityId_)) {
						return sol::make_object(luaState, &em->GetComponent<SphereColliderData>(this->bindEntityId_));
					}
				}
			}
			// 環境のフォールバック（グローバルなど）をチェック
			return luaState.globals()[key];
		};
		// sol::table::set_metatable がない古いバージョン向けの互換記法
		env_table[sol::metatable_key] = mt;

		// 2. 作成したテーブルから環境を初期化

		environment_ = sol::environment(luaState, env_table);
		// 環境の親としてglobalsを設定（globalsから値を取得できるようにする）
		// ただし、メタテーブルの__indexで手動フォールバックしているので実質的にはmt経由でアクセスされる
		
		std::string filePath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + scriptName;
		sol::load_result loadResult = luaState.load_file(filePath);

		if (!loadResult.valid()) {
			sol::error err = loadResult;
			throw std::runtime_error("Failed to load Lua script: " + scriptName + "\n" + err.what());
		}

		SetQFEFunctions();

		// スクリプトを実行（環境を指定）
		sol::function scriptFunc = loadResult;
		sol::set_environment(environment_, scriptFunc);

		sol::protected_function_result execResult = scriptFunc();
		if (!execResult.valid()) {
			sol::error err = execResult;
			throw std::runtime_error("Failed to execute Lua script: " + scriptName + "\n" + err.what());
		}

		isCanRun_ = true;
		scriptName_ = scriptName;

		// キャッシュする関数を取得
		initFunc_ = environment_["Init"];
		updateFunc_ = environment_["Update"];
		onCollisionEnterFunc_ = environment_["OnCollisionEnter"];
		onCollisionStayFunc_ = environment_["OnCollisionStay"];

		// Lua側のレジストリに登録
		if (updateFunc_.valid()) {
			luaState["QFE_Internal"]["RegisterUpdate"](handle_, updateFunc_, priority_);
		}
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
	return &LuaScriptResourceManager::GetInstance()->GetSharedState();
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
	sol::state& luaState = LuaScriptResourceManager::GetInstance()->GetSharedState();

	// thisエンティティ情報登録
	sol::table thisEntity = luaState.create_table();
	thisEntity.set_function("GetEntityId", [this]() {
		return bindEntityId_;
		});
	environment_["this"] = thisEntity;

	// 訳アリ関数群
	// Log
	environment_["DebugLog"] = [this](sol::variadic_args message) {
		message;
#ifdef _DEBUG
		DebugLogLua(message,this->GetBindEntityId(),this->GetScriptName());
#endif // _DEBUG
		};
	environment_["GetThisEntityId"] = [this]() {
		return bindEntityId_;
		};
	environment_["RunEntityScriptFunction"] = 
		[](uint32_t entityId, const std::string& scriptName, const std::string& functionName) {
			LuaScriptResourceManager::GetInstance()->RunFunction(entityId, scriptName, functionName);
		};
	environment_["destroy"] = [this]() {
		SceneManager::GetInstance()->DeleteEntity(this->GetBindEntityId());
		};
	environment_["delete"] = [this]() {
		SceneManager::GetInstance()->DeleteEntity(this->GetBindEntityId());
		};
	environment_["GetEntityScriptGlobal"] = 
		[this](uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::this_state ts) {
			sol::state_view callerState(ts);
			return LuaScriptResourceManager::GetInstance()->GetEntityScriptGlobal(entityId, scriptName, varName, callerState);
		};
	environment_["SetEntityScriptGlobal"] = 
		[this](uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::object value) {
			LuaScriptResourceManager::GetInstance()->SetEntityScriptGlobal(entityId, scriptName, varName, value);
		};

	environment_["SetAABBColiderSize"] = [this](const Vector3& size) {
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<AABBColliderData>(this->GetBindEntityId()) == false) {
			return;
		}
		AABBColliderData& collider = entityManager->GetComponent<AABBColliderData>(this->GetBindEntityId());
		collider.aabb.size = size;
		};
}

