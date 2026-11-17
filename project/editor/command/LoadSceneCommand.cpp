#include "LoadSceneCommand.h"
#include "scene/SceneManager.h"

#include "ecs/entity/EntityManager.h"

QFE::EDITOR::LoadSceneCommand::LoadSceneCommand(const std::string& scenePath, QFE::SCENE::SceneManager* sceneManager):
	sceneManager_(sceneManager), scenePath_(scenePath) {
}

void QFE::EDITOR::LoadSceneCommand::Execute() {
	previousScene_ = sceneManager_->CreateSnapshot();
	sceneManager_->LoadCurrentSceneFromJson(scenePath_);
}

void QFE::EDITOR::LoadSceneCommand::Undo() {
	sceneManager_->RestoreSnapshot(previousScene_);
}
