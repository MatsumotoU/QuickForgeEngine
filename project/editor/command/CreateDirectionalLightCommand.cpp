#include "CreateDirectionalLightCommand.h"
#include "ecs/entity/EntityManager.h"
#include "framework/scene/SceneCoreFramework.h"

QFE::EDITOR::CreateDirectionalLightCommand::CreateDirectionalLightCommand(
	QFE::SCENE::SceneManager* sceneManager) : sceneManager_(sceneManager), entityId_(UINT32_MAX) {}

void QFE::EDITOR::CreateDirectionalLightCommand::Execute() {
	if (sceneManager_ != nullptr) entityId_ = QFE::FRAMEWORK::CreateSceneDirectionalLight(*sceneManager_);
}

void QFE::EDITOR::CreateDirectionalLightCommand::Undo() {
	if (sceneManager_ != nullptr && entityId_ != UINT32_MAX) {
		QFE::FRAMEWORK::GetSceneEntities(*sceneManager_).RemoveEntity(entityId_);
	}
}
