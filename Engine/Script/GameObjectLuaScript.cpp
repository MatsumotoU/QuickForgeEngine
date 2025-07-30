#include "GameObjectLuaScript.h"
#include "Model/Model.h"
#include "Scene/SceneObject.h"
#include "Object/BaseGameObject.h"
#include <cassert>

#include "Base/EngineCore.h"
#include "Input/XInput/XInputController.h"

GameObjectLuaScript::GameObjectLuaScript(BaseGameObject* obj, EngineCore* engineCore) {
	// Model型のバインド
	GetLuaState().new_usertype<Model>("Model",
		sol::no_constructor,
		"GetModelFileName", &Model::GetModelFileName,
		"SetBlendmode", &Model::SetBlendmode,
		"LoadModel", &Model::LoadModel,
		"Draw", &Model::Draw,
		"Init", &Model::Init,
		"Update", &Model::Update
	);

	// SceneObject型のバインド
	GetLuaState().new_usertype<SceneObject>("SceneObject",
		sol::no_constructor,
		"GetGameObjects", [](SceneObject& self) -> std::vector<BaseGameObject*> {
			std::vector<BaseGameObject*> result;
			for (auto& obj : self.GetGameObjects()) {
				result.push_back(obj.get());
			}
			return result;
		},
		"AddModel", &SceneObject::AddModel,
		"DeleteModel", &SceneObject::DeleteModel,
		"GetSceneName", &SceneObject::GetSceneName
	);

	// BaseGameObject型のバインド（必要に応じて）
	GetLuaState().new_usertype<BaseGameObject>("BaseGameObject",
		sol::no_constructor,
		"GetWorldPosition", &BaseGameObject::GetWorldPosition,
		"GetName", &BaseGameObject::GetName,
		"SetName", &BaseGameObject::SetName,
		"localPos", &BaseGameObject::localPos_,
		"transform", &BaseGameObject::transform_,
		"worldPos", &BaseGameObject::worldPos_,
		"worldMatrix", &BaseGameObject::worldMatrix_
	);

	// Luaのthisを設定
	assert(obj != nullptr && "GameObjectLuaScript: obj is null");
	GetLuaState()["this"] = obj;

	// コントローラー登録
	assert(engineCore != nullptr && "GameObjectLuaScript: engineCore is null");
	GetLuaState().new_usertype<XInputController>("XInputController",
		"GetIsActiveController", &XInputController::GetIsActiveController,
		"GetPressButton", &XInputController::GetPressButton,
		"GetLeftStick", &XInputController::GetLeftStick,
		"GetRightStick", &XInputController::GetRightStick
	);
	GetLuaState()["controller"] = engineCore->GetXInputController();

	// DirectInputManager型のバインド
		GetLuaState().new_usertype<DirectInputManager>("DirectInputManager",
			"GetKeyMoveDir", &DirectInputManager::GetKeyMoveDir
		);
	// Luaグローバルにinputとして登録
	GetLuaState()["input"] = engineCore->GetInputManager();
}

void GameObjectLuaScript::LoadScript(const std::string& scriptFilePath) {
	// ファイル存在チェック
	if (!std::filesystem::exists(scriptFilePath)) {
#ifdef _DEBUG
		DebugLog(("LuaScript file not found: " + scriptFilePath).c_str());
#endif
		return;
	}
	try {
		GetLuaState().script_file(scriptFilePath);
	}
	catch (const sol::error& e) {
#ifdef _DEBUG
		DebugLog(("LuaScript load error: " + std::string(e.what())).c_str());
		assert(false && "LuaScript load error");
#endif
	}
}

void GameObjectLuaScript::Init() {
	sol::function luaFunc = luaState_["Init"];
	if (!luaFunc.valid()) {
		assert(false && "Update is not found");
	}
	luaFunc(); // 存在すれば実行
}

void GameObjectLuaScript::Update() {
	sol::function updateFunc = GetLuaState()["Update"];
	if (!updateFunc.valid()) {
		assert(false && "Update function is not found in Lua script");
		return;
	}
	updateFunc();
}