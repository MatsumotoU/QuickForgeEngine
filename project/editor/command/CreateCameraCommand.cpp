#include "CreateCameraCommand.h"

#include "ecs/entity/EntityManager.h"
#include "framework/scene/SceneCoreFramework.h"

QFE::EDITOR::CreateCameraCommand::CreateCameraCommand(QFE::SCENE::SceneManager* sceneManager) :
	sceneManager_(sceneManager), entityId_(UINT32_MAX) {
}

void QFE::EDITOR::CreateCameraCommand::Execute() {
	if (sceneManager_ == nullptr) {
		return;
	}
	entityId_ = QFE::FRAMEWORK::CreateSceneCamera(*sceneManager_);
}

void QFE::EDITOR::CreateCameraCommand::Undo() {
	if (sceneManager_ == nullptr || entityId_ == UINT32_MAX) {
		return;
	}
	QFE::FRAMEWORK::GetSceneEntities(*sceneManager_).RemoveEntity(entityId_);
}
