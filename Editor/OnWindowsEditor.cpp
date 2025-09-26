#include "OnWindowsEditor.h"

#ifdef _DEBUG
#include "Assets/AssetManager.h"
#include "AppUtility/FileSystems/FileUtility.h"
#include "AppUtility/DebugTool/ImGui/FrameController/ImGuiFlameController.h"
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

OnWindowsEditor::OnWindowsEditor() {

}

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
