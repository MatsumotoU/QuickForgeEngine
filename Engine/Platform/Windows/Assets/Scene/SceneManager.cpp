#include "SceneManager.h"
#include "SceneObject.h"

#include "Assets/AssetManager.h"
#include "Assets/Camera/CameraManager.h"
#include "Assets/3DModel/Data/ModelHandle.h"

void SceneManager::Initalize() {
	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();
}

void SceneManager::Update() {
	currentScene_->Update();
}

void SceneManager::Draw() {
	currentScene_->Draw();
}

void SceneManager::Finalize() {
}

void SceneManager::ResetScene() {
	AssetManager::GetInstance()->GetEntityManager()->ResetEntiry();
	CameraManager::GetInstance()->Initialize();
}

void SceneManager::LoadModel(const std::string& modelName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	ModelHandle modelHandle;
	modelHandle.handle = assetManager->LoadModel(modelName);
	assetManager->GetEntityManager()->EmplaceComponent<ModelHandle>(entityId, modelHandle);

}
