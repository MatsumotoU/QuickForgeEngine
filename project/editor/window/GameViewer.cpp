#include "GameViewer.h"
#include "AssetDrop.h"

QFE::EDITOR::GameViewer::GameViewer(ImTextureID sceneTextureId, SCENE::SceneManager* sceneManager, unsigned width, unsigned height) :
	sceneTextureId_(sceneTextureId),
	sceneManager_(sceneManager), width_(width), height_(height),
	isActive_(true) {
}

void QFE::EDITOR::GameViewer::Initialize() {
	isActive_ = true;
}

void QFE::EDITOR::GameViewer::Draw(std::set<uint32_t>&, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_);
	const float targetAspectRatio = static_cast<float>(width_) / static_cast<float>(height_);
	isFocus_ = ImGui::IsWindowFocused();

	// 2. ウィンドウ内で「実際に利用可能な最大サイズ」を取得
	ImVec2 availSize = ImGui::GetContentRegionAvail();
	if (availSize.x <= 0 || availSize.y <= 0) { ImGui::End(); return; }

	// 3. アスペクト比を維持した最終的な描画サイズを計算
	ImVec2 renderSize;
	if (availSize.x / availSize.y > targetAspectRatio) {
		// ウィンドウが横に長すぎる場合 ➡️ 縦幅（高さ）を基準にして横幅を決める
		renderSize.y = availSize.y;
		renderSize.x = availSize.y * targetAspectRatio;
	} else {
		// ウィンドウが縦に長すぎる場合 ➡️ 横幅を基準にして縦幅（高さ）を決める
		renderSize.x = availSize.x;
		renderSize.y = availSize.x / targetAspectRatio;
	}

	// 4. 画像をウィンドウの中央に寄せるための「余白（黒帯）」を計算
	ImVec2 offsetX = ImVec2((availSize.x - renderSize.x) * 0.5f, 0.0f);
	ImVec2 offsetY = ImVec2(0.0f, (availSize.y - renderSize.y) * 0.5f);

	// 5. カーソル位置をずらして中央配置にする
	if (offsetX.x > 0.0f) {
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX.x);
	}
	if (offsetY.y > 0.0f) {
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY.y);
	}

	// 6. ID を使って描画
	const ImVec2 imagePosition = ImGui::GetCursorScreenPos();
	ImGui::Image(sceneTextureId_, renderSize);
	AcceptAssetDrop(sceneManager_, commandList, imagePosition, renderSize, width_, height_,
		MATH::Matrix4x4::MakeIdentity4x4(), dropViewProjection_);

	ImGui::End();
}

void QFE::EDITOR::GameViewer::SetAssetDropViewProjection(const MATH::Matrix4x4& viewProjection) {
	dropViewProjection_ = viewProjection;
}

std::string QFE::EDITOR::GameViewer::GetWindowName() {
	return "GameViewer";
}

bool QFE::EDITOR::GameViewer::GetIsActive() {
	return isActive_;
}

bool QFE::EDITOR::GameViewer::SetIsActive(bool isActive) {
	isActive_ = isActive;
	return isActive_;
}

bool QFE::EDITOR::GameViewer::GetIsFocus() {
	return isFocus_;
}
