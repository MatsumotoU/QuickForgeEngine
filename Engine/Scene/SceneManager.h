#pragma once
#include "SceneObject.h"

class EngineCore;

class SceneManager {
public:
	void CreateScene(EngineCore* engineCore, const std::string& sceneName);

public:
	void InitializeScene();
	void UpdateScene();
	void DrawScene();

public:
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

public:
	std::string GetCurrentSceneName() const;

private:
	std::unique_ptr<SceneObject> currentScene_;
	EngineCore* engineCore_;

private:
#ifdef _DEBUG
	int modelSelectionIndex_;
	std::vector<std::string> modelFilepaths_;
	std::string inputFilepath_;
	std::string ModelDirectoryPath_;
#endif // _DEBUG

};