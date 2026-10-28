#include "CreateDirectionalLightCommand.h"
#include "design-patterns/EntityManager.h"
#include "scene/SceneManager.h"

QFE::EDITOR::CreateDirectionalLightCommand::CreateDirectionalLightCommand(
	QFE::SCENE::SceneManager* sceneManager) : sceneManager_(sceneManager), entityId_(UINT32_MAX) {}

void QFE::EDITOR::CreateDirectionalLightCommand::Execute() {
	if (sceneManager_ != nullptr) entityId_ = sceneManager_->CreateDirectionalLightEntity();
}

void QFE::EDITOR::CreateDirectionalLightCommand::Undo() {
	if (sceneManager_ != nullptr && entityId_ != UINT32_MAX) {
		sceneManager_->GetCurrentSceneEntityManager().RemoveEntity(entityId_);
	}
}
