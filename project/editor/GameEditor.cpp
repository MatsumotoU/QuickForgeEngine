#include "GameEditor.h"
#include "design-patterns/EntityManager.h"
#include "scene/SceneManager.h"

using namespace QFE::EDITOR;

void QFE::EDITOR::GameEditor::Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow) {
	windowManager_.Initialize(sceneManager, sceneTextureId, mainWindow);
	commandExecutor_.Initialize();
	commandList_.ClearCommands();

	activeCameraType_ = EditorCameraType::DebugCamera;
}

void GameEditor::Update() {
	commandList_.ClearCommands();

	// シーンビューがアクティブになった場合デバッグカメラに切り替える
	if(windowManager_.IsWindowFocused(EditorWindowType::SceneViewer)) {
		activeCameraType_ = EditorCameraType::DebugCamera;
	}
}

void GameEditor::Draw() {
	windowManager_.Draw(commandList_);
	commandExecutor_.ExecuteCommand(&commandList_);
}