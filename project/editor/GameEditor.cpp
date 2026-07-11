#include "GameEditor.h"
#include "design-patterns/EntityManager.h"
#include "scene/SceneManager.h"

using namespace QFE::EDITOR;

void QFE::EDITOR::GameEditor::Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow) {
	windowManager_.Initialize(sceneManager, sceneTextureId, mainWindow);
	commandExecutor_.Initialize();
	commandList_.ClearCommands();
}

void GameEditor::Update() {
	commandList_.ClearCommands();
}

void GameEditor::Draw() {
	windowManager_.Draw(commandList_);
	commandExecutor_.ExecuteCommand(&commandList_);
}
