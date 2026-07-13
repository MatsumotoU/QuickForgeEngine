#include "LoadSceneCommand.h"
#include "scene/SceneManager.h"

#include "framework/window/WindowsWindowFrameWork.h"
#include "design-patterns/EntityManager.h"

QFE::EDITOR::LoadSceneCommand::LoadSceneCommand(const std::string& scenePath, QFE::SCENE::SceneManager* sceneManager):
	sceneManager_(sceneManager), scenePath_(scenePath) {
}

void QFE::EDITOR::LoadSceneCommand::Execute() {
	previousSceneData_ = sceneManager_->GetCurrentSceneEntityManager().Serialize();
	sceneManager_->GetCurrentSceneEntityManager().ResetEntity();
	sceneManager_->LoadCurrentSceneFromJson(scenePath_);
}

void QFE::EDITOR::LoadSceneCommand::Undo() {
	sceneManager_->GetCurrentSceneEntityManager().ResetEntity();
	sceneManager_->GetCurrentSceneEntityManager().DeserializeEntityComponents(0, previousSceneData_);
}
