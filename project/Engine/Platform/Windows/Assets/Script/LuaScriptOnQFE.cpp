#include "LuaScriptOnQFE.h"
#include "Core/EngineGlobalValue.h"
#include "Assets/AssetManager.h"
#include "Scene/SceneManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Input/InputInterface.h"
#include "Core/Math/Transform.h"
#include "Physics/Force.h"
#include "Assets/Script/Data/ScriptHandle.h"
#include "Scene/Data/SceneObjectData.h"
#include "Assets/Sprite/Data/SpriteData.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
#include "Core/Math/MyMath.h"

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

		SetQFEFunctions();

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

	// transformコンポーネントをLuaにセット
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

	// ForceコンポーネントをLuaにセット
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
	// テーブル作成
	sol::table qfe = luaState_->create_named_table("QFE");
	sol::table input = qfe.create_named("Input");
	sol::table math = qfe.create_named("Math");

	luaState_->set_function("LookAtFromDir", [](const Vector3& dir) {
		Vector3 lookAt;
		float yaw = atan2f(dir.x, dir.z);
		float pitch = asinf(-dir.y / dir.Length());
		lookAt.y = yaw;
		lookAt.x = pitch;
		return lookAt;
		});

	// * 関数登録 * //
	math.set("pi", 3.14159265358979323846f);
	math.set_function("Leap", [](float a, float b, float t) {
		return a * t + b * (1.0f - t);
		});
	math.set_function("EaseIn", [](float from, float to, float t) {
		return MyMath::EaseIn(from, to, t);
		});
	math.set_function("EaseOut", [](float from, float to, float t) {
		return MyMath::EaseOut(from, to, t);
		});
	math.set_function("EaseInOut", [](float from, float to, float t) {
		return MyMath::EaseInOut(from, to, t);
		});
	math.set_function("Rand", [](sol::object minObj, sol::object maxObj) {
		float min = 0.0f;
		float max = 1.0f;
		if (minObj.is<int>()) {
			min = static_cast<float>(minObj.as<int>());
		} else if (minObj.is<double>()) {
			min = static_cast<float>(minObj.as<double>());
		} else if (minObj.is<float>()) {
			min = minObj.as<float>();
		}
		if (maxObj.is<int>()) {
			max = static_cast<float>(maxObj.as<int>());
		} else if (maxObj.is<double>()) {
			max = static_cast<float>(maxObj.as<double>());
		} else if (maxObj.is<float>()) {
			max = maxObj.as<float>();
		}
		return MyMath::Rand(min, max);
		});
	
	InputInterface* inputManager = InputInterface::GetInstance();
	input.set_function("GetKeyMoveDir", [inputManager]() {
		return inputManager->GetKeyMoveDir();
		});

	// キーボード
	input.set_function("GetKeyPress", [inputManager](std::string actionName) {
		return inputManager->GetKeyPress(actionName);
		});
	input.set_function("GetKeyTrigger", [inputManager](std::string actionName) {
		return inputManager->GetKeyTrigger(actionName);
		});
	input.set_function("GetKeyRelease", [inputManager](std::string actionName) {
		return inputManager->GetKeyRelease(actionName);
		});

	// マウス
	input.set_function("GetMousePress", [inputManager](int8_t button) {
		return inputManager->GetMousePress(button);
		});
	input.set_function("GetMouseTrigger", [inputManager](int8_t button) {
		return inputManager->GetMouseTrigger(button);
		});
	input.set_function("GetMouseRelease", [inputManager](int8_t button) {
		return inputManager->GetMouseRelease(button);
		});
	input.set_function("GetMouseScreenPos", [inputManager]() {
		return inputManager->GetMouseScreenPos();
		});
	input.set_function("GetMouseMoveDir", [inputManager]() {
		return inputManager->GetMouseMove();
		});
	input.set_function("GetMouseWheelDir", [inputManager]() {
		return inputManager->GetMouseWheelDir();
		});

#ifdef _DEBUG
	luaState_->set_function("DebugLog", [](sol::variadic_args message) {
		DebugLogLua(message);
		});
#endif // _DEBUG
	luaState_->new_usertype<SceneObjectData>("SceneObjectData",
		"name", &SceneObjectData::name,
		"tag", &SceneObjectData::tag
	);

	luaState_->set_function("GetEntity", [](const std::string& entityName) {
		return SceneManager::GetInstance()->GetEntityByName(entityName);
		});
	luaState_->set_function("GetEntityFromUniqeID", [](uint32_t uniqeId) {
		return SceneManager::GetInstance()->GetEntityByUniqeID(uniqeId);
		}
	);
	luaState_->set_function("GetIsDraw", [](uint32_t entityId) {
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<SpriteData>(entityId)) {
			SpriteData& sprite = entityManager->GetComponent<SpriteData>(entityId);
			return sprite.isDraw;
		}
		return false;
	});
	luaState_->set_function("SetIsDraw", [](uint32_t entityId,bool isDraw) {
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<SpriteData>(entityId)) {
			SpriteData& sprite = entityManager->GetComponent<SpriteData>(entityId);
			sprite.isDraw = isDraw;
		}
		});
	luaState_->set_function("GetTransform",
		[](uint32_t entityId) {
			AssetManager* assetManager = AssetManager::GetInstance();
			return &assetManager->GetEntityManager()->GetComponent<Transform>(entityId);
		}
	);

	luaState_->set_function("GetEntityScriptGlobal",
		[this](uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::this_state ts) {
			sol::state_view callerState(ts);
			return LuaScriptResourceManager::GetInstance()->GetEntityScriptGlobal(entityId, scriptName, varName, callerState);
		}
	);

	luaState_->set_function("CreateEntity", [](const std::string& entityName,const Transform& transform) {
		uint32_t id = SceneManager::GetInstance()->RunTimeAddEntity(entityName);
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<Transform>(id)) {
			Transform& t = entityManager->GetComponent<Transform>(id);
			t = transform;
		}
		return id;
		});

	// GlobalValue
	luaState_->set_function("destroy", [this]() {
		AssetManager::GetInstance()->GetEntityManager()->RemoveEntity(this->GetBindEntityId());
		});

	luaState_->set_function("LoadScene", [](const std::string& sceneName) {
		SceneManager::GetInstance()->RunTimeSwapScene(sceneName);
		});

	// Math
	luaState_->new_usertype<Vector2>("Vector2",
		sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y,

		"Length", &Vector2::Length,
		"Normalize", & Vector2::Normalize
	);
	luaState_->new_usertype<Vector3>("Vector3",
		sol::constructors<Vector3(), Vector3(float, float, float)>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z,

		"Length", &Vector3::Length,
		"Normalize", sol::resolve<Vector3() const>(&Vector3::Normalize)
	);
	luaState_->new_usertype<Vector4>("Vector4",
		sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
		"x", &Vector4::x,
		"y", &Vector4::y,
		"z", &Vector4::z,
		"w", &Vector4::w,

		"Length", &Vector4::Length,
		"Normalize", & Vector4::Normalize
	);
	luaState_->new_usertype<Transform>("Transform",
		sol::constructors<Transform()>(),
		"scale", &Transform::scale,
		"rotate", &Transform::rotate,
		"translate", &Transform::translate,
		"AddForward", &Transform::AddForward,
		"AddRight", &Transform::AddRight
	);

	luaState_->new_usertype<Force>("Force",
		sol::constructors<Force()>(),
		"velocity", &Force::velocity,
		"acceleration", &Force::acceleration,
		"mass", &Force::mass,
		"friction", &Force::friction,
		"gravityStrength", &Force::gravityStrength,
		"isGravity", &Force::isGravity
	);

	sol::table dik = luaState_->create_named_table("DIK");
	dik["W"] = static_cast<uint32_t>(0x11);
	dik["A"] = static_cast<uint32_t>(0x1E);
	dik["S"] = static_cast<uint32_t>(0x1F);
	dik["D"] = static_cast<uint32_t>(0x20);
	dik["SPACE"] = static_cast<uint32_t>(0x39);
	// 必要なキーを追加
	luaState_->set("DIK_W", 0x11);
	luaState_->set("DIK_A", 0x1E);
	luaState_->set("DIK_S", 0x1F);
	luaState_->set("DIK_D", 0x20);
	luaState_->set("DIK_SPACE", 0x39);
	luaState_->set("DIK", dik);

	luaState_->set_function("GetTransform", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		return em->HasComponent<Transform>(entityId) ? &em->GetComponent<Transform>(entityId) : nullptr;
		});
	luaState_->set_function("GetSceneObjectData", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		return em->HasComponent<SceneObjectData>(entityId) ? &em->GetComponent<SceneObjectData>(entityId) : nullptr;
		});
	luaState_->set_function("GetForce", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		return em->HasComponent<Force>(entityId) ? &em->GetComponent<Force>(entityId) : nullptr;
		});
}