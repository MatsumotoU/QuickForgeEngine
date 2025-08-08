#include "SceneManager.h"
#include "Base/EngineCore.h"
#include <utility>
#include "Utility/FileLoader.h"
#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

static char saveFileName[256] = "";
static int selectedIndex = -1;

//#include "Script/Lua/LuaScriptGenerator.h"
//
//static char newLuaFileName[128] = "NewScript.lua";
//static std::string lastCreatedLuaPath;
//static bool showCreateResult = false;
//
//static std::string lastScriptRunScenePath;

SceneManager::SceneManager() {
	isRequestSwapScene_ = false;
	loadedScenePath_.clear();
}

void SceneManager::CreateScene(EngineCore* engineCore, const std::string& sceneName) {
	engineCore_ = engineCore;
	currentScene_ = std::make_unique<SceneObject>(engineCore, sceneName);
	sceneDataDirectorypath_ = "Resources/Scenes";
	sceneFilepath_.clear();
	sceneSelectionIndex_ = 0;
	sceneFilepath_ = FileLoader::GetFilesWithExtension(sceneDataDirectorypath_, ".json");
	if (sceneFilepath_.size() > 0) {
		selectedSceneFilepath_ = sceneFilepath_[0]; // 最初のシーンを選択
	}

#ifdef _DEBUG
	requestRedo_ = false;
	requestUndo_ = false;
#endif // _DEBUG
}

void SceneManager::InitializeScene() {
#ifndef _DEBUG
	isRequestSwapScene_ = true;
	loadedScenePath_ = "Resources/Scenes/LastScriptRunScene.json";
	//startToRunScript_ = true;
#endif // !_DEBUG

	if (currentScene_) {
		currentScene_->Initialize();
	}
}

void SceneManager::UpdateScene() {
#ifdef _DEBUG
	/*Undo();
	Redo();*/
#endif // _DEBUG

#ifndef _DEBUG
	if (!isRunningScript_) {
		isRunningScript_ = true;
	}
#endif // !_DEBUG

	// シーンの実行
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::DrawScene() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::SaveScenesToJson(const std::string& filepath) {
#ifdef _DEBUG
	DebugLog("===Start save scenes!===");
#endif // _DEBUG

	nlohmann::json root;
	/*if (currentScene_) {
		root["scenes"].push_back(currentScene_->Serialize());
	}*/

	std::ofstream ofs(filepath);
	if (!ofs) {
		// ディレクトリがなければ作成
		std::filesystem::path path(filepath);
		std::filesystem::create_directories(path.parent_path());
		// 再度ファイルを開く
		ofs.open(filepath);
	}
	assert(ofs);
	ofs << root.dump(4); // インデント付きで出力
	ofs.close();

#ifdef _DEBUG
	DebugLog("===End save scenes===");
#endif // _DEBUG
}

void SceneManager::LoadScenesFromJson(const std::string& filepath) {
#ifdef _DEBUG
	DebugLog(std::format("LoadScene: {}", filepath));
#endif // _DEBUG
	loadedScenePath_ = filepath;
	isRequestSwapScene_ = true;
	engineCore_->GetDirectXCommon()->WaitForGpu();
}

void SceneManager::LoadScenesLua(const std::string& filename) {
	LoadScenesFromJson(sceneDataDirectorypath_ + "/" + filename + ".json");
}

void SceneManager::SwapScene() {
//	if (isRequestSwapScene_) {
//		isRequestSwapScene_ = false;
//
//		engineCore_->GetLuaScriptManager()->ClearAllGameObjScripts();
//
//		std::ifstream ifs;
//		assert(!loadedScenePath_.empty() && "Loaded scene path is empty");
//		ifs.open(loadedScenePath_);
//		assert(ifs);
//
//		nlohmann::json root;
//		ifs >> root;
//		if (root.contains("scenes")) {
//			loadedScene_ = SceneObject::Deserialize(root["scenes"][0], engineCore_, ModelDirectoryPath_);
//		}
//		assert(loadedScene_ && "Loaded scene is null");
//
//		currentScene_ = std::move(loadedScene_);
//		currentScene_->Initialize();
//
//		// ここでcurrentScene_のGameObjectとScriptの状態をデバッグ出力
//#ifdef _DEBUG
//		for (const auto& obj : currentScene_->GetGameObjects()) {
//			DebugLog(("GameObject: " + obj->GetName() + ", Script: " + obj->GetAttachedScriptName()).c_str());
//		}
//#endif
//		engineCore_->GetChiptune()->PlayNoise();
//	}
}

std::string SceneManager::GetCurrentSceneName() const {
	return currentScene_ ? currentScene_->GetSceneName() : "UnnamedScene";
}