#include "LuaScriptOnQFE.h"
#include "Core/EngineGlobalValue.h"
#include "Assets/AssetManager.h"
#include "Scene/SceneManager.h"
#include "Input/DirectInput/DirectInputManager.h"
#include "Core/Math/Transform.h"
#include "Assets/Script/Data/ScriptHandle.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

LuaScriptOnQFE::LuaScriptOnQFE() {
	isCanRun_ = false;
	scriptName_ = "";
	bindEntityId_ = 0;
	defaultGlobals.clear();
	UserGlobals.clear();
}

void LuaScriptOnQFE::LoadScript(const std::string& scriptName) {
	isCanRun_ = false;
	try {
		luaState_ = std::make_unique<sol::state>();
		luaState_->open_libraries(sol::lib::base, sol::lib::package);

		// 起動直後のグローバル一覧を保存
		for (auto& kv : luaState_->globals()) {
			defaultGlobals.insert(kv.first.as<std::string>());
		}

		std::string filePath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + scriptName;
		sol::load_result loadResult = luaState_->load_file(filePath);

		if (!loadResult.valid()) {
			sol::error err = loadResult;
			throw std::runtime_error("Failed to load Lua script: " + scriptName + "\n" + err.what());
		}

		// スクリプトを実行
		sol::protected_function_result execResult = loadResult();
		if (!execResult.valid()) {
			sol::error err = execResult;
			throw std::runtime_error("Failed to execute Lua script: " + scriptName + "\n" + err.what());
		}

		// Userグローバル一覧を保存（スクリプト実行後！）
		for (auto& kv : luaState_->globals()) {
			if (defaultGlobals.find(kv.first.as<std::string>()) == defaultGlobals.end()) {
				UserGlobals.insert(kv.first.as<std::string>());
			}
		}

		SetQFEFunctions();
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
	// 古いグローバル変数を新しいスクリプトにコピー
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

void LuaScriptOnQFE::RunFunction(const std::string& functionName) {
	luaState_->set("deltaTime", QFE::EngineGlobalValue::deltaTime);

	try {
		if (!isCanRun_) {
			throw std::runtime_error("Cannot run function. Lua script is not loaded or failed to load.");
		}
		if (!luaState_) {
			throw std::runtime_error("Lua state is not initialized.");
		}
		sol::function func = luaState_->get<sol::function>(functionName);
		if (!func) {
			throw std::runtime_error("Function " + functionName + " not found in Lua script.");
		}
		sol::protected_function_result result = func();
		if (!result.valid()) {
			sol::error err = result;
			throw std::runtime_error("Error running function " + functionName + ": " + std::string(err.what()));
		}
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(e.what(), LogLevel::Error);
#else
		std::cerr << e.what() << std::endl;
#endif
	}
}

bool LuaScriptOnQFE::HasFunction(const std::string& functionName) const {
	if (luaState_) {
		sol::object obj = luaState_->get<sol::object>(functionName);
		return obj.is<sol::function>();
	}
	return false;
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
	return luaState_.get();
}

const bool& LuaScriptOnQFE::IsCanRun() const {
	return isCanRun_;
}

bool LuaScriptOnQFE::IsAliveEntity() {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	if (entityManager->HasComponent<ScriptHandles>(bindEntityId_)) {
		return true;
	}
	return false;
}

void LuaScriptOnQFE::SetEntityValue(uint32_t entityId) {
	bindEntityId_ = entityId;
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	// transformコンポーネントをLuaにセット
	try
	{
		if (entityManager->HasComponent<Transform>(entityId))
		{
			Transform& transform = entityManager->GetComponent<Transform>(entityId);
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
	// テーブル作成
	sol::table qfe = luaState_->create_named_table("QFE");
	sol::table input = qfe.create_named("Input");

	// * 関数登録 * //
	DirectInputManager* inputManager = DirectInputManager::GetInstance();
	input.set_function("GetKeyMoveDir", [inputManager]() {
		return inputManager->GetKeyMoveDir();
		});
#ifdef _DEBUG
	luaState_->set_function("DebugLog", [](sol::variadic_args message) {
		DebugLogLua(message);
		});
#endif // _DEBUG

	luaState_->set_function("CreateEntity", [](const std::string& entityName) {
		return SceneManager::GetInstance()->AddEntity(entityName);
		});

	luaState_->set_function("SetPosition", [this](const Vector3& position, uint32_t entityId) {
		SetPosition(entityId, position);
		});
	luaState_->set_function("SetPosition", [this](sol::table posTable, uint32_t entityId) {
		Vector3 position;
		position.x = posTable[1].get_or(0.0f);
		position.y = posTable[2].get_or(0.0f);
		position.z = posTable[3].get_or(0.0f);
		SetPosition(entityId, position);
		});

	// GlobalValue
	

	// Math
	luaState_->new_usertype<Vector2>("Vector2",
		sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y
	);
	luaState_->new_usertype<Vector3>("Vector3",
		sol::constructors<Vector3(), Vector3(float, float, float)>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z
	);
	luaState_->new_usertype<Vector4>("Vector4",
		sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
		"x", &Vector4::x,
		"y", &Vector4::y,
		"z", &Vector4::z,
		"w", &Vector4::w
	);
	luaState_->new_usertype<Transform>("Transform",
		sol::constructors<Transform()>(),
		"scale", &Transform::scale,
		"rotate", &Transform::rotate,
		"translate", &Transform::translate
	);
}

void LuaScriptOnQFE::SetPosition(uint32_t entityId, const Vector3& position) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (entityManager->HasComponent<Transform>(entityId)) {
		Transform& transform = entityManager->GetComponent<Transform>(entityId);
		transform.translate = position;
	} else {
#ifdef _DEBUG
		DebugLog("Entity does not have Transform component.", LogLevel::Error);
#endif // _DEBUG
	}
}
