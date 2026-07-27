#include "SceneViewer.h"
#include "command/ChangeTransformCommand.h"
#include "command/EditorCommandList.h"
#include "design-patterns/EntityManager.h"
#include "components/TransformComponent.h"
#include "scene/SceneManager.h"

QFE::EDITOR::SceneViewer::SceneViewer(ImTextureID sceneTextureId, SCENE::SceneManager* sceneManager) :
	sceneManager_(sceneManager),
	sceneTextureId_(sceneTextureId),
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
	const float targetAspectRatio = 16.0f / 9.0f;
	isFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

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

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::SetRect(imagePosition.x, imagePosition.y, renderSize.x, renderSize.y);
	ImGuizmo::SetOrthographic(isOrthographic_);

	DrawGizmoToolbar(isImageHovered_);
	DrawGizmo(selectedEntities, commandList);

	ImGui::End();
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
	EntityManager& entityManager = sceneManager_->GetCurrentSceneEntityManager();
	if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
		wasUsingGizmo_ = false;
		return;
	}

	auto& transformComponent =
		entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
	QFE::MATH::Matrix4x4 transformMatrix =
		QFE::MATH::Matrix4x4::MakeAffineMatrix(transformComponent.transform);

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
		transformComponent.transform.FromMatrix(transformMatrix);
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
