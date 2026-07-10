#include "GameEditor.h"
#include "design-patterns/EntityManager.h"

using namespace QFE::EDITOR;

void QFE::EDITOR::GameEditor::Initialize(QFE::EntityManager* entityManager, ImTextureID sceneTextureId) {
	windowManager_.Initialize(entityManager, sceneTextureId);
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
