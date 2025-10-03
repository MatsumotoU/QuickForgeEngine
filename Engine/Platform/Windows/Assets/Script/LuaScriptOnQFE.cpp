#include "LuaScriptOnQFE.h"
#include "Core/EngineGlobalValue.h"
#include "Assets/AssetManager.h"
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
		// Userグローバル一覧を保存
		for (auto& kv : luaState_->globals()) {
			if (defaultGlobals.find(kv.first.as<std::string>()) == defaultGlobals.end()) {
				UserGlobals.insert(kv.first.as<std::string>());
			}
		}
		if (!loadResult.valid()) {
			sol::error err = loadResult;
			throw std::runtime_error("Failed to load Lua script: " + scriptName + "\n" + err.what());
		}

		sol::protected_function_result execResult = loadResult();
		if (!execResult.valid()) {
			sol::error err = execResult;
			throw std::runtime_error("Failed to execute Lua script: " + scriptName + "\n" + err.what());
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
	luaState_->set_function("DebugLog", [](const std::string& message) {
		DebugLog(message,LogLevel::EditorInfo);
		});
#endif // _DEBUG

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
