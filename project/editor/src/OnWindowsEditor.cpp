#include "editor/include/OnWindowsEditor.h"

#ifdef _DEBUG
#include "engine/include/assets/AssetManager.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
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
