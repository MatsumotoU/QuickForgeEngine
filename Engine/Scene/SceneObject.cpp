#include "SceneObject.h"
#include "Base/EngineCore.h"
#include "Model/Model.h"

SceneObject::SceneObject(EngineCore* enginecore, const std::string& sceneName) {
	sceneName_ = sceneName;
	engineCore_ = enginecore;
	mainCamera_.Initialize(enginecore->GetWinApp());
#ifdef _DEBUG
	debugCamera_.Initialize(enginecore);
#endif // _DEBUG

	enginecore->GetGraphRenderer()->SetCamera(&mainCamera_);
}

SceneObject::~SceneObject() {
	engineCore_->GetGraphRenderer()->DeleteCamera();
}

void SceneObject::Initialize() {
	// Initialize the scene
	for (const auto& gameObject : gameObjects_) {
		gameObject->Init();
	}
}

void SceneObject::Update() {
	mainCamera_.Update();
#ifdef _DEBUG
	debugCamera_.Update();
	mainCamera_ = debugCamera_.camera_;
#endif // _DEBUG

	// Update the scene
	for (const auto& gameObject : gameObjects_) {
		gameObject->Update();
	}
}

void SceneObject::Draw() {
#ifdef _DEBUG
	engineCore_->GetGraphRenderer()->DrawGrid(50.0f, 50);
#endif // _DEBUG

	// Draw the scene
	for (const auto& gameObject : gameObjects_) {
		gameObject->Draw(&mainCamera_);
	}
}

void SceneObject::AddModel(const std::string& directoryPath, const std::string& filename) {
	Model model(engineCore_);
	model.LoadModel(directoryPath, filename,COORDINATESYSTEM_HAND_RIGHT);
	model.Init();
	gameObjects_.push_back(std::make_unique<Model>(model));
}

IScene* SceneObject::GetNextScene() {
	return nullptr;
}
