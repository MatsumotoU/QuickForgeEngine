#include "GameViewer.h"
#include "AssetDrop.h"
#include "components/CameraComponent.h"
#include "components/CameraRoomComponent.h"
#include "components/ObjectInfoComponent.h"
#include "components/TransformHierarchy.h"
#include "framework/scene/SceneCoreFramework.h"
#include <algorithm>
#include <vector>

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

	// 部屋の選択はエディター内の一時状態で、シーンやカメラには保存しない。
	auto& entities = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
	struct RoomItem { std::string uuid; std::string name; };
	std::vector<RoomItem> rooms;
	entities.Each<SCENE::CameraRoomComponent, SCENE::ObjectInfoComponent>(
		[&](uint32_t id, SCENE::CameraRoomComponent&, SCENE::ObjectInfoComponent& info) {
			if (!info.uuid.empty() && entities.HasComponent<SCENE::TransformComponent>(id))
				rooms.push_back({ info.uuid, info.name.empty() ? "Unnamed Room" : info.name });
		});
	std::sort(rooms.begin(), rooms.end(), [](const RoomItem& a, const RoomItem& b) {
		return a.name == b.name ? a.uuid < b.uuid : a.name < b.name;
	});
	const auto selected = std::find_if(rooms.begin(), rooms.end(), [&](const RoomItem& room) { return room.uuid == previewRoomUuid_; });
	if (!previewRoomUuid_.empty() && selected == rooms.end()) previewRoomUuid_.clear();
	const char* label = previewRoomUuid_.empty() ? "Main Camera" : selected->name.c_str();
	ImGui::SetNextItemWidth(220.0f);
	if (ImGui::BeginCombo("Camera Room Preview", label)) {
		if (ImGui::Selectable("Main Camera", previewRoomUuid_.empty())) previewRoomUuid_.clear();
		for (const auto& room : rooms) {
			ImGui::PushID(room.uuid.c_str());
			if (ImGui::Selectable(room.name.c_str(), previewRoomUuid_ == room.uuid)) previewRoomUuid_ = room.uuid;
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

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

bool QFE::EDITOR::GameViewer::TryGetRoomPreview(MATH::Matrix4x4& viewProjection, MATH::Vector3& cameraPosition) const {
	if (previewRoomUuid_.empty() || !sceneManager_ || !isActive_) return false;
	auto& entities = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
	uint32_t roomId;
	if (!SCENE::TryGetEntityIdByUuid(entities, previewRoomUuid_, roomId) ||
		!entities.HasComponent<SCENE::CameraRoomComponent>(roomId) ||
		!entities.HasComponent<SCENE::TransformComponent>(roomId)) return false;
	const auto world = SCENE::GetWorldMatrix(entities, roomId);
	const auto& room = entities.GetComponent<SCENE::CameraRoomComponent>(roomId);
	const MATH::Vector3 position = MATH::Vector3{ world.m[3][0], world.m[3][1], world.m[3][2] } + room.cameraOffset;
	bool foundCamera = false;
	entities.Each<SCENE::CameraComponent>([&](uint32_t id, SCENE::CameraComponent& camera) {
		if (!camera.isMainCamera || !entities.HasComponent<SCENE::TransformComponent>(id)) return;
		// UpdateMainCamera が求めた回転と投影はそのまま使い、位置だけを一時的に差し替える。
		auto cameraWorld = camera.viewMatrix.Inverse();
		cameraWorld.m[3][0] = position.x;
		cameraWorld.m[3][1] = position.y;
		cameraWorld.m[3][2] = position.z;
		viewProjection = MATH::Matrix4x4::Multiply(cameraWorld.Inverse(), camera.projectionMatrix);
		cameraPosition = position;
		foundCamera = true;
	});
	return foundCamera;
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
