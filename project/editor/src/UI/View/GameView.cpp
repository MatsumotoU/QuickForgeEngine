/**
 * @file GameView.cpp
 * @brief ゲームの最終描画結果を表示するパネルの実装
 */

#include "editor/include/UI/View/GameView.h"

#include "engine/include/graphic/PostEffect/RenderingPostprocess.h"
#include "engine/include/camera/CameraManager.h"
using namespace QFE;
GameView::GameView() {
	SetName("Game View");
	isActive_ = true;
	isSceneViewFocused_ = false;
}

void GameView::Initialize() {
}

void GameView::Update() {
	if (isSceneViewFocused_) {
#ifdef QFE_OPTIMIZE_OFF
		CameraManager::GetInstance()->SetActiveDebugCamera(false);
#endif // _DEBUG
	}
}

void GameView::Draw() {
#ifdef QFE_OPTIMIZE_OFF
	if (!isActive_) {
		return;
	}

	RenderingPostprocess* render = RenderingPostprocess::GetInstance();
	DescriptorHandles handle = render->GetCurrentSrvHandle();
	ImGui::Begin(GetName().c_str());

	// フォーカス判定
	isSceneViewFocused_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 contentRegionMin = ImGui::GetWindowContentRegionMin();
	ImVec2 contentRegionMax = ImGui::GetWindowContentRegionMax();

	ImVec2 contentMin = ImVec2(windowPos.x + contentRegionMin.x, windowPos.y + contentRegionMin.y);
	ImVec2 contentMax = ImVec2(windowPos.x + contentRegionMax.x, windowPos.y + contentRegionMax.y);
	ImVec2 contentSize = ImVec2(contentMax.x - contentMin.x, contentMax.y - contentMin.y);

	float targetAspect = 1280.0f / 720.0f;
	ImVec2 imageSize;
	if (contentSize.x / contentSize.y > targetAspect) {
		imageSize.y = contentSize.y;
		imageSize.x = contentSize.y * targetAspect;
	} else {
		imageSize.x = contentSize.x;
		imageSize.y = contentSize.x / targetAspect;
	}
	ImVec2 centerPos = ImVec2(
		contentMin.x + (contentSize.x - imageSize.x) * 0.5f,
		contentMin.y + (contentSize.y - imageSize.y) * 0.5f
	);
	ImGui::SetCursorScreenPos(centerPos);
	ImGui::Image((void*)handle.gpuHandle_.ptr, imageSize);
	ImGui::End();
#endif // _DEBUG
}
