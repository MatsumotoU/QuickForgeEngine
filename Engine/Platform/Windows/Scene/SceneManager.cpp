#include "SceneManager.h"
#include "SceneObject.h"

#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Assets/3DModel/Data/ModelHandle.h"
#include "Data/SceneObjectData.h"

void SceneManager::Initalize() {
	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();

	CameraManager::GetInstance()->Initialize();
}

void SceneManager::Update() {
	currentScene_->Update();
}

void SceneManager::PreDraw() {
	// カメラ更新
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();
	AssetManager* assetManager = AssetManager::GetInstance();
	for (uint32_t i = 0; i < assetManager->GetWpvBufferManager()->GetBufferCount(); i++) {
		Camera& camera = cameraManager->GetMainCamera();
		TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(i);
		wpvMatrix->WVP = camera.GetWorldViewProjectionMatrix(wpvMatrix->World);
	}
}

void SceneManager::Draw() {
	currentScene_->Draw();
}

void SceneManager::PostDraw() {
}

void SceneManager::Finalize() {
	CameraManager::GetInstance()->Shutdown();
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
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, SceneObjectData{ modelName, "Untagged" });
}
