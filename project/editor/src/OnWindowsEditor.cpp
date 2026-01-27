/**
 * @file OnWindowsEditor.cpp
 * @brief Windowsプラットフォーム向けエディタの実装
 */

#include "editor/include/OnWindowsEditor.h"

#ifdef _DEBUG
#include "engine/include/assets/AssetManager.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
using namespace QFE;

OnWindowsEditor::OnWindowsEditor() {

}

/** @brief 初期化 */
void OnWindowsEditor::Initialize() {
#ifdef _DEBUG
	// デバッグビルド時のみUIマネージャーを初期化
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
