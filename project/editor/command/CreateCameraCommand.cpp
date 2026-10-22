#include "CreateCameraCommand.h"

#include "design-patterns/EntityManager.h"
#include "scene/SceneManager.h"

QFE::EDITOR::CreateCameraCommand::CreateCameraCommand(QFE::SCENE::SceneManager* sceneManager) :
	sceneManager_(sceneManager), entityId_(UINT32_MAX) {
}

void QFE::EDITOR::CreateCameraCommand::Execute() {
	if (sceneManager_ == nullptr) {
		return;
	}
	entityId_ = sceneManager_->CreateCameraEntity();
}

void QFE::EDITOR::CreateCameraCommand::Undo() {
	if (sceneManager_ == nullptr || entityId_ == UINT32_MAX) {
		return;
	}
	sceneManager_->GetCurrentSceneEntityManager().RemoveEntity(entityId_);
}
