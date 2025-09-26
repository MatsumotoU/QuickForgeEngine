#include "GameView.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"
#include "Assets/Camera/CameraManager.h"

GameView::GameView() {
	name_ = "Game View";
	isActive_ = true;
	isSceneViewFocused_ = false;
}

void GameView::Initialize() {
}

void GameView::Update() {
	if (isSceneViewFocused_) {
		CameraManager::GetInstance()->SetActiveDebugCamera(false);
	}
}

void GameView::Draw() {
	if (!isActive_) {
		return;
	}

	RendaringPostprosecess* render = RendaringPostprosecess::GetInstance();
	DescriptorHandles handle = render->GetCurrentSrvHandle();
	ImGui::Begin(name_.c_str());

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

}
