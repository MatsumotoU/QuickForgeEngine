#include "OnWindowsEditor.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/ImGui/FrameController/ImGuiFlameController.h"
#endif // _DEBUG

void OnWindowsEditor::Initialize() {
#ifdef _DEBUG
	uiManager_.Initialize();
#endif // _DEBUG
}

void OnWindowsEditor::Update() {
#ifdef _DEBUG
	uiManager_.Update();
#endif // _DEBUG
}

void OnWindowsEditor::Draw() {
#ifdef _DEBUG
	uiManager_.Draw();
#endif // _DEBUG
}
