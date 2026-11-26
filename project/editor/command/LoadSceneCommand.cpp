#include "LoadSceneCommand.h"
#include "framework/scene/SceneCoreFramework.h"

#include "ecs/entity/EntityManager.h"

QFE::EDITOR::LoadSceneCommand::LoadSceneCommand(const std::string& scenePath, QFE::SCENE::SceneManager* sceneManager):
	sceneManager_(sceneManager), scenePath_(scenePath) {
}

void QFE::EDITOR::LoadSceneCommand::Execute() {
	previousScene_ = QFE::FRAMEWORK::CaptureScene(*sceneManager_);
	QFE::FRAMEWORK::LoadScene(*sceneManager_, scenePath_);
}

void QFE::EDITOR::LoadSceneCommand::Undo() {
	QFE::FRAMEWORK::RestoreScene(*sceneManager_, previousScene_);
}
