#include "CreateNewScene.h"
#include "Assets/Scene/SceneManager.h"

void CreateNewScene::Initialize() {
	name_ = "New Scene";
	isActive_ = false;
}

void CreateNewScene::Update() {
}

void CreateNewScene::Draw() {
}

void CreateNewScene::Run() {
	SceneManager::GetInstance()->ResetScene();
}
