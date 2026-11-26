#include "SaveSceneCommand.h"
#include "framework/scene/SceneCoreFramework.h"
#include "ecs/entity/EntityManager.h"

#include "file/FileUtility.h"

QFE::EDITOR::SaveSceneCommand::SaveSceneCommand(const std::string& savePath, QFE::SCENE::SceneManager* sceneManager) :
	sceneManager_(sceneManager),
	scenePath_(savePath),
	previousSceneData_() {
}

void QFE::EDITOR::SaveSceneCommand::Execute() {
	// ファイルを上書き保存する前に、現在のシーンの状態をJSONとして保存しておく
	if (QFE::FILE::HasFile(scenePath_)) {
		previousSceneData_ = QFE::FRAMEWORK::ReadSceneJson(*sceneManager_, scenePath_);
	}
	QFE::FRAMEWORK::SaveScene(*sceneManager_, scenePath_);
}

void QFE::EDITOR::SaveSceneCommand::Undo() {
	// シーンの保存を取り消す場合、以前のシーンデータを復元する
	if (!previousSceneData_.is_null()) {
		QFE::FRAMEWORK::LoadScene(*sceneManager_, previousSceneData_);
	}
	QFE::FRAMEWORK::SaveScene(*sceneManager_, scenePath_);
}
