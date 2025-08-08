#pragma once
#include "SceneObject.h"
#include <stack>
#include "Utility/SimpleJson.h"

#ifdef _DEBUG
#include "Base/DirectX/ImGuiManager.h"
#endif // _DEBUG

#include "Colliders/CollisionManager.h"

class EngineCore;

// TODO: シーンでルアスクリプトを作らない
// TODO: シーンマネージャーはシーンの切り替えとシーンの保存・読み込みのみを行う

class SceneManager {
public:
	SceneManager();

public:
	void CreateScene(EngineCore* engineCore, const std::string& sceneName);

public:
	void InitializeScene();
	void UpdateScene();
	void DrawScene();

public:
	void SaveScenesToJson(const std::string& filepath);
	void LoadScenesFromJson(const std::string& filepath);
	void LoadScenesLua(const std::string& filename);
	void SwapScene();

public:
	std::string GetCurrentSceneName() const;

private:
	EngineCore* engineCore_;
#ifdef _DEBUG
	/*void Undo();
	void Redo();
	void PushUndo();*/
	bool requestUndo_;
	bool requestRedo_;
	std::stack<nlohmann::json> undoStack_;
	std::stack<nlohmann::json> redoStack_;
#endif // _DEBUG
	nlohmann::json currentSceneData_;
	bool isRequestSwapScene_; // シーンの切り替え要求フラグ
	std::string loadedScenePath_;
	std::unique_ptr<SceneObject> loadedScene_;
	std::unique_ptr<SceneObject> currentScene_;
	std::vector<std::string> sceneFilepath_;
	std::string selectedSceneFilepath_;
	int sceneSelectionIndex_; // 現在選択されているシーンのインデックス
	std::string sceneDataDirectorypath_;
};