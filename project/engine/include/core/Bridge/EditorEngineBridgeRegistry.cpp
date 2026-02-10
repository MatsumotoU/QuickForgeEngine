#include "EditorEngineBridgeRegistry.h"
#include "engine/include/WindowsEngineCore.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"

#include "engine/include/scene/Data/SceneObjectData.h"

void QFE::EditorEngineBridgeRegistry::RegisterFunctions(WindowsEngineCore* engineCore) {
#ifdef QFE_OPTIMIZE_OFF
	EditorEngineBridge::GetModelDirectoryPath = [engineCore]() -> std::string {
		return engineCore->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Model");
		};
	EditorEngineBridge::GetImageDirectoryPath = [engineCore]() -> std::string {
		return engineCore->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Image");
		};
	EditorEngineBridge::GetEntityTemplateDirectoryPath = [engineCore]() -> std::string {
		return engineCore->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
		};

	EditorEngineBridge::GetAllEntityIds = [engineCore]() -> std::vector<uint32_t> {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			return sceneManager_->GetEntityManager()->GetActiveEntityIds();
		}
		return {};
		};
	EditorEngineBridge::GetEntityName = [engineCore](uint32_t entityId) -> std::string {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			if (sceneManager_->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				SceneObjectData& data = sceneManager_->GetEntityManager()->GetComponent<SceneObjectData>(entityId);
				return data.name;
			} else {
				DebugLog(std::format("Entity ID {} does not have SceneObjectData component.", entityId));
			}
		}
		return "";
		};

	EditorEngineBridge::AddEmptyEntity = [engineCore]() {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddEmptyObject();
		}
		};
	EditorEngineBridge::AddEntityFromFile = [engineCore](const std::string& filePath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddEntity(filePath);
		}
		};
	EditorEngineBridge::AddModelEntity = [engineCore](const std::string& modelPath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddModel(modelPath);
		}
		};
	EditorEngineBridge::AddSpriteEntity = [engineCore](const std::string& spritePath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddSprite(spritePath);
		}
		};
	EditorEngineBridge::AddParticleEmitterEntity = [engineCore](const std::string& particlePath, uint32_t count) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddParticleEmitter(particlePath, count);
		}
		};
	EditorEngineBridge::AddCameraEntity = [engineCore]() {
		// カメラの機能がシングルトンであるため、複数カメラにすると不具合が起きる可能性があるため一時的に制限
		DebugLog("Can not add Camera.");
		};
	EditorEngineBridge::CopyEntity = [engineCore](uint32_t entityId) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->CopyEntity(entityId);
		}
		};
	EditorEngineBridge::SaveEntity = [engineCore](uint32_t entityId, std::string filePath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->SaveEntity(entityId, filePath);
		}
		};
	EditorEngineBridge::DeleteEntity = [engineCore](uint32_t entityId) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->DeleteEntity(entityId);
		}
		};
	EditorEngineBridge::ParentChild = [engineCore](uint32_t parentId, uint32_t childId) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->ParentChild(parentId, childId);
		}
		};

	EditorEngineBridge::GetDebugCameraEntityId = [engineCore]() -> uint32_t {
		if (!CameraManager::GetInstance()) {
			DebugLog("CameraManager is not initialized.");
			return UINT32_MAX;
		}
		return CameraManager::GetInstance()->GetCamera(0).GetBindEntityId();
		};
#endif // QFE_OPTIMIZE_OFF
}

void QFE::EditorEngineBridgeRegistry::ClearFunctions() {
#ifdef QFE_OPTIMIZE_OFF
	EditorEngineBridge::GetModelDirectoryPath = nullptr;
	EditorEngineBridge::GetImageDirectoryPath = nullptr;
	EditorEngineBridge::GetEntityTemplateDirectoryPath = nullptr;
	EditorEngineBridge::GetAllEntityIds = nullptr;
	EditorEngineBridge::GetEntityName = nullptr;
	EditorEngineBridge::AddEmptyEntity = nullptr;
	EditorEngineBridge::AddEntityFromFile = nullptr;
	EditorEngineBridge::AddModelEntity = nullptr;
	EditorEngineBridge::AddSpriteEntity = nullptr;
	EditorEngineBridge::AddParticleEmitterEntity = nullptr;
	EditorEngineBridge::AddCameraEntity = nullptr;
	EditorEngineBridge::CopyEntity = nullptr;
	EditorEngineBridge::SaveEntity = nullptr;
	EditorEngineBridge::DeleteEntity = nullptr;
	EditorEngineBridge::ParentChild = nullptr;
#endif // QFE_OPTIMIZE_OFF
}
