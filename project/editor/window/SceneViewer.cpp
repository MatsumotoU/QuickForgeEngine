#include "SceneViewer.h"
#include "AssetDrop.h"
#include "command/ChangeTransformCommand.h"
#include "command/EditorCommandList.h"
#include "ecs/entity/EntityManager.h"
#include "components/TransformComponent.h"
#include "components/TransformHierarchy.h"
#include "framework/scene/SceneCoreFramework.h"
#include <cmath>

namespace {
	// 同次クリップ空間で線分を切り、カメラ背面や画面外への線の伸びを防ぐ。
	bool ClipGridLine(QFE::MATH::Vector4& a, QFE::MATH::Vector4& b) {
		const auto distance = [](const QFE::MATH::Vector4& v, int plane) {
			switch (plane) {
			case 0: return v.w + v.x;
			case 1: return v.w - v.x;
			case 2: return v.w + v.y;
			case 3: return v.w - v.y;
			case 4: return v.z;
			default: return v.w - v.z;
			}
		};
		for (int plane = 0; plane < 6; ++plane) {
			const float da = distance(a, plane), db = distance(b, plane);
			if (!std::isfinite(da) || !std::isfinite(db) || (da < 0.0f && db < 0.0f)) return false;
			if (da < 0.0f) a = a + (b - a) * (da / (da - db));
			else if (db < 0.0f) b = b + (a - b) * (db / (db - da));
		}
		return a.w > 1.0e-6f && b.w > 1.0e-6f;
	}
}

QFE::EDITOR::SceneViewer::SceneViewer(ImTextureID sceneTextureId, SCENE::SceneManager* sceneManager, unsigned width, unsigned height) :
	sceneManager_(sceneManager),
	sceneTextureId_(sceneTextureId),
	width_(width), height_(height),
	isActive_(true),
	isFocus_(false),
	isImageHovered_(false),
	isOrthographic_(false),
	wasUsingGizmo_(false),
	editingEntityId_(0),
	gizmoOperation_(ImGuizmo::TRANSLATE),
	gizmoMode_(ImGuizmo::LOCAL),
	viewMatrix_(QFE::MATH::Matrix4x4::MakeIdentity4x4()),
	projectionMatrix_(QFE::MATH::Matrix4x4::MakeIdentity4x4()) {
}

void QFE::EDITOR::SceneViewer::Initialize() {
	isActive_ = true;
}

void QFE::EDITOR::SceneViewer::Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_);
	const float targetAspectRatio = static_cast<float>(width_) / static_cast<float>(height_);
	isFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	DrawGridControls();

	ImVec2 availSize = ImGui::GetContentRegionAvail();
	if (availSize.x <= 0.0f || availSize.y <= 0.0f) {
		isImageHovered_ = false;
		ImGui::End();
		return;
	}

	ImVec2 renderSize;
	if (availSize.x / availSize.y > targetAspectRatio) {
		renderSize.y = availSize.y;
		renderSize.x = availSize.y * targetAspectRatio;
	} else {
		renderSize.x = availSize.x;
		renderSize.y = availSize.x / targetAspectRatio;
	}

	const ImVec2 contentStart = ImGui::GetCursorScreenPos();
	const ImVec2 imagePosition(
		contentStart.x + (availSize.x - renderSize.x) * 0.5f,
		contentStart.y + (availSize.y - renderSize.y) * 0.5f);
	ImGui::SetCursorScreenPos(imagePosition);
	ImGui::Image(sceneTextureId_, renderSize);
	isImageHovered_ = ImGui::IsItemHovered();
	AcceptAssetDrop(sceneManager_, commandList, imagePosition, renderSize, width_, height_,
		MATH::Matrix4x4::MakeIdentity4x4(), dropViewProjection_, &grid_);
	if (grid_.enabled) DrawPlacementGrid(imagePosition, renderSize);

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::SetRect(imagePosition.x, imagePosition.y, renderSize.x, renderSize.y);
	ImGuizmo::SetOrthographic(isOrthographic_);

	DrawGizmoToolbar(isImageHovered_);
	DrawGizmo(selectedEntities, commandList);

	ImGui::End();
}

void QFE::EDITOR::SceneViewer::DrawGridControls() {
	ImGui::Checkbox("Grid Snap", &grid_.enabled);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Snap asset placement and gizmo translation to the world grid.");
	if (!grid_.enabled) return;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(78.0f);
	const char* names[] = { "XZ", "XY", "YZ" };
	int plane = static_cast<int>(grid_.plane);
	if (ImGui::Combo("Plane", &plane, names, IM_ARRAYSIZE(names))) grid_.plane = static_cast<GridPlane>(plane);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("Spacing", &grid_.spacing, 0.05f, 0.01f, 1000.0f, "%.2f")) {
		grid_.spacing = std::isfinite(grid_.spacing) ? std::clamp(grid_.spacing, 0.01f, 1000.0f) : 1.0f;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(75.0f);
	const char* layerLabel = grid_.plane == GridPlane::XZ ? "Y Layer" : grid_.plane == GridPlane::XY ? "Z Layer" : "X Layer";
	if (ImGui::DragInt(layerLabel, &grid_.layer, 1.0f, -1000, 1000)) {
		grid_.layer = std::clamp(grid_.layer, -1000, 1000);
	}
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Plane offset = layer x spacing, along the remaining world axis.");
}

void QFE::EDITOR::SceneViewer::DrawPlacementGrid(const ImVec2& imagePosition, const ImVec2& imageSize) {
	MATH::Vector3 center{};
	// 中央に面が見えなければ原点付近を表示する。配置可否はドロップ位置で別に判定する。
	IntersectGrid(grid_, dropViewProjection_, 0.5f, 0.5f, center);
	const float height = static_cast<float>(grid_.layer) * grid_.spacing;
	const auto point = [&](float u, float v) -> MATH::Vector3 {
		if (grid_.plane == GridPlane::XZ) return { u, height, v };
		if (grid_.plane == GridPlane::XY) return { u, v, height };
		return { height, u, v };
	};
	const float centerU = grid_.plane == GridPlane::YZ ? center.y : center.x;
	const float centerV = grid_.plane == GridPlane::XY ? center.y : center.z;
	const float extent = 20.0f * grid_.spacing;
	auto* draw = ImGui::GetWindowDrawList();
	draw->PushClipRect(imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y), true);
	const auto line = [&](const MATH::Vector3& start, const MATH::Vector3& end, bool major) {
		auto a = MATH::Vector4::EulerTransform({ start.x, start.y, start.z, 1.0f }, dropViewProjection_);
		auto b = MATH::Vector4::EulerTransform({ end.x, end.y, end.z, 1.0f }, dropViewProjection_);
		if (!ClipGridLine(a, b)) return;
		const auto screen = [&](const MATH::Vector4& p) {
			return ImVec2(imagePosition.x + (p.x / p.w + 1.0f) * 0.5f * imageSize.x,
				imagePosition.y + (1.0f - p.y / p.w) * 0.5f * imageSize.y);
		};
		draw->AddLine(screen(a), screen(b), major ? IM_COL32(160, 190, 220, 190) : IM_COL32(150, 170, 190, 105), major ? 1.5f : 1.0f);
	};
	for (int index = -20; index <= 20; ++index) {
		const float u = centerU + static_cast<float>(index) * grid_.spacing;
		const float v = centerV + static_cast<float>(index) * grid_.spacing;
		line(point(u, centerV - extent), point(u, centerV + extent), index % 10 == 0);
		line(point(centerU - extent, v), point(centerU + extent, v), index % 10 == 0);
	}
	const auto* payload = ImGui::GetDragDropPayload();
	const ImVec2 mouse = ImGui::GetMousePos();
	if (payload && payload->IsDataType(AssetPayloadType) &&
		mouse.x >= imagePosition.x && mouse.y >= imagePosition.y &&
		mouse.x < imagePosition.x + imageSize.x && mouse.y < imagePosition.y + imageSize.y) {
		MATH::Vector3 placement{};
		if (IntersectGrid(grid_, dropViewProjection_, (mouse.x - imagePosition.x) / imageSize.x,
			(mouse.y - imagePosition.y) / imageSize.y, placement)) {
			const auto projected = MATH::Vector4::EulerTransform({ placement.x, placement.y, placement.z, 1.0f }, dropViewProjection_);
			if (projected.w > 1.0e-6f) {
				const ImVec2 marker(imagePosition.x + (projected.x / projected.w + 1.0f) * 0.5f * imageSize.x,
					imagePosition.y + (1.0f - projected.y / projected.w) * 0.5f * imageSize.y);
				draw->AddCircleFilled(marker, 5.0f, IM_COL32(255, 210, 65, 240));
			}
		} else {
			ImGui::SetTooltip("The selected plane is parallel to the view or outside the camera range.");
		}
	}
	draw->PopClipRect();
}

std::string QFE::EDITOR::SceneViewer::GetWindowName() {
	return "SceneViewer";
}

bool QFE::EDITOR::SceneViewer::GetIsActive() {
	return isActive_;
}

bool QFE::EDITOR::SceneViewer::SetIsActive(bool isActive) {
	isActive_ = isActive;
	return isActive_;
}

bool QFE::EDITOR::SceneViewer::GetIsFocus() {
	return isFocus_;
}

void QFE::EDITOR::SceneViewer::SetCameraMatrices(
	const QFE::MATH::Matrix4x4& viewMatrix,
	const QFE::MATH::Matrix4x4& projectionMatrix,
	bool isOrthographic) {
	viewMatrix_ = viewMatrix;
	projectionMatrix_ = projectionMatrix;
	isOrthographic_ = isOrthographic;
}

bool QFE::EDITOR::SceneViewer::IsGizmoCapturingMouse() const {
	return isImageHovered_ && (ImGuizmo::IsUsing() || ImGuizmo::IsOver());
}

void QFE::EDITOR::SceneViewer::SetAssetDropViewProjection(const MATH::Matrix4x4& viewProjection) {
	dropViewProjection_ = viewProjection;
}

void QFE::EDITOR::SceneViewer::DrawGizmoToolbar(bool imageHovered) {
	const ImGuiIO& io = ImGui::GetIO();
	if (imageHovered && !io.WantTextInput) {
		if (ImGui::IsKeyPressed(ImGuiKey_W)) {
			gizmoOperation_ = ImGuizmo::TRANSLATE;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_E)) {
			gizmoOperation_ = ImGuizmo::ROTATE;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_R)) {
			gizmoOperation_ = ImGuizmo::SCALE;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_X) && gizmoOperation_ != ImGuizmo::SCALE) {
			gizmoMode_ = gizmoMode_ == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
		}
	}

	if (imageHovered && ImGui::IsKeyPressed(ImGuiKey_LeftAlt)) {
		ImGui::OpenPopup("SceneGizmoMenu");
	}
	if (ImGui::BeginPopup("SceneGizmoMenu")) {
		if (ImGui::MenuItem("Translate (W)", nullptr, gizmoOperation_ == ImGuizmo::TRANSLATE)) {
			gizmoOperation_ = ImGuizmo::TRANSLATE;
		}
		if (ImGui::MenuItem("Rotate (E)", nullptr, gizmoOperation_ == ImGuizmo::ROTATE)) {
			gizmoOperation_ = ImGuizmo::ROTATE;
		}
		if (ImGui::MenuItem("Scale (R)", nullptr, gizmoOperation_ == ImGuizmo::SCALE)) {
			gizmoOperation_ = ImGuizmo::SCALE;
		}
		ImGui::Separator();
		const bool localMode = gizmoMode_ == ImGuizmo::LOCAL;
		if (ImGui::MenuItem("Local (X)", nullptr, localMode, gizmoOperation_ != ImGuizmo::SCALE)) {
			gizmoMode_ = ImGuizmo::LOCAL;
		}
		if (ImGui::MenuItem("World (X)", nullptr, !localMode, gizmoOperation_ != ImGuizmo::SCALE)) {
			gizmoMode_ = ImGuizmo::WORLD;
		}
		ImGui::EndPopup();
	}
}

void QFE::EDITOR::SceneViewer::DrawGizmo(
	std::set<uint32_t>& selectedEntities,
	EditorCommandList& commandList) {
	if (selectedEntities.size() != 1 || sceneManager_ == nullptr) {
		wasUsingGizmo_ = false;
		return;
	}

	const uint32_t entityId = *selectedEntities.begin();
	EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
	if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
		wasUsingGizmo_ = false;
		return;
	}

	auto& transformComponent =
		entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
	QFE::MATH::Matrix4x4 transformMatrix =
		QFE::SCENE::GetWorldMatrix(entityManager, entityId);

	ImGuizmo::SetID(static_cast<int>(entityId));
	const bool changed = ImGuizmo::Manipulate(
		&viewMatrix_.m[0][0],
		&projectionMatrix_.m[0][0],
		gizmoOperation_,
		gizmoOperation_ == ImGuizmo::SCALE ? ImGuizmo::LOCAL : gizmoMode_,
		&transformMatrix.m[0][0]);
	const bool isUsingGizmo = ImGuizmo::IsUsing();

	if (isUsingGizmo && !wasUsingGizmo_) {
		editingEntityId_ = entityId;
		transformBeforeEdit_ = transformComponent.transform;
	}

	if (changed && isUsingGizmo) {
		if (grid_.enabled && gizmoOperation_ == ImGuizmo::TRANSLATE) {
			// 親の変換を戻す前にワールド位置を丸め、親子の有無で格子をずらさない。
			const auto snapped = SnapGridPosition({ transformMatrix.m[3][0], transformMatrix.m[3][1], transformMatrix.m[3][2] }, grid_.spacing);
			transformMatrix.m[3][0] = snapped.x;
			transformMatrix.m[3][1] = snapped.y;
			transformMatrix.m[3][2] = snapped.z;
		}
		const QFE::MATH::Matrix4x4 parentWorldMatrix =
			QFE::SCENE::GetParentWorldMatrix(entityManager, entityId);
		const QFE::MATH::Matrix4x4 localMatrix =
			QFE::MATH::Matrix4x4::Multiply(transformMatrix, parentWorldMatrix.Inverse());
		transformComponent.transform.FromMatrix(localMatrix);
	}

	if (!isUsingGizmo && wasUsingGizmo_ && editingEntityId_ == entityId) {
		const QFE::MATH::EulerTransform transformAfterEdit = transformComponent.transform;
		if (transformAfterEdit != transformBeforeEdit_) {
			commandList.AddCommand(std::make_unique<ChangeTransformCommand>(
				entityId,
				&entityManager,
				transformBeforeEdit_,
				transformAfterEdit));
		}
	}

	wasUsingGizmo_ = isUsingGizmo;
}
