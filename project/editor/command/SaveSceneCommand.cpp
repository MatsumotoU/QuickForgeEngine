#include "SaveSceneCommand.h"
#include "scene/SceneManager.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "design-patterns/EntityManager.h"

#include "file/FileUtility.h"

QFE::EDITOR::SaveSceneCommand::SaveSceneCommand(const std::string& savePath, SCENE::SceneManager* sceneManager) :
	scenePath_(savePath),
	sceneManager_(sceneManager),
	previousSceneData_() {
}

void QFE::EDITOR::SaveSceneCommand::Execute() {
	// ファイルを上書き保存する前に、現在のシーンの状態をJSONとして保存しておく
	if (QFE::FILE::HasFile(scenePath_)) {
		previousSceneData_ = sceneManager_->LoadCurrentSceneToJson(scenePath_);
	}
	sceneManager_->SaveCurrentSceneToJson(scenePath_);
}

void QFE::EDITOR::SaveSceneCommand::Undo() {
	// シーンの保存を取り消す場合、以前のシーンデータを復元する
	if (!previousSceneData_.is_null()) {
		sceneManager_->LoadCurrentSceneFromJson(previousSceneData_);
	}
	sceneManager_->SaveCurrentSceneToJson(scenePath_);
}
