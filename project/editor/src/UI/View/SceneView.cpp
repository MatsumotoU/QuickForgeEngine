#include "editor/include/UI/View/SceneView.h"
#include "editor/include/UI/View/HierarchyView.h"

#include "engine/include/graphic/PostEffect/RendaringPostprocess.h"
#include "engine/include/camera/CameraManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h" 
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#include "engine/include/InputInterface.h"
#include "engine/include/renderer/GraphRenderer.h"
#include "engine/include/core/Math/MyMath.h"

#include <cmath>
#include <algorithm>

SceneView::SceneView() {
	name_ = "Scene View";
	isActiveCamera_ = true;
	isDrawGrid_ = true;

	anchorPoint_ = { 0.0f,0.0f,0.0f };
	mouseSensitivity_ = 0.5f;
	cameraMoveT_ = 1.0f;
	targetRotate_ = { 0.0f,0.0f,0.0f };
	startPos_ = targetRotate_;
	isDrawGizmo_ = true;
}

void SceneView::Initialize() {
	isActive_ = true;
}

void SceneView::Update() {
#ifdef _DEBUG
	selectEntityId_ = HierarchyView::selectedEntityId_;

	DebugCameraControl();
#endif // _DEBUG
}

void SceneView::Draw() {
	if (!isActive_) {
		return;
	}
#ifdef _DEBUG
	RendaringPostprosecess* render = RendaringPostprosecess::GetInstance();
	DescriptorHandles handle = render->GetCurrentSrvHandle();
	ImGui::Begin("Scene View");

	// 繝輔か繝ｼ繧ｫ繧ｹ蛻､螳・
	bool isSceneViewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	if (isSceneViewFocused) {
		isActiveCamera_ = true;
	} else {
		isActiveCamera_ = false;
	}

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
	ImGuizmo::SetRect(centerPos.x, centerPos.y, imageSize.x, imageSize.y);
	UpdateGizmo();
	ImGui::End();

	if (isDrawGrid_) {
		GraphRenderer::GetInstance()->DrawGrid();
	}
#endif // _DEBUG
}

void SceneView::DebugCameraControl() {
#ifdef _DEBUG
	Camera& camera = CameraManager::GetInstance()->GetCamera(0);
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<Transform>(camera.GetBindEntityId())) {
		return;
	}
	Transform& cameraTransform = entityManager->GetComponent<Transform>(camera.GetBindEntityId());

	if (isActiveCamera_) {
		CameraManager::GetInstance()->SetActiveDebugCamera(true);
		DirectInputManager& input = InputInterface::GetInstance()->GetDirectInputManager();

		float distance = (cameraTransform.translate - anchorPoint_).Length();
		if (input.keyboard_.GetTrigger(DIK_NUMPAD7)) {
			targetRotate_ = { distance, 0.0f, 3.14f };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (input.keyboard_.GetTrigger(DIK_NUMPAD1)) {
			targetRotate_ = { distance, -3.14f * 0.5f,3.14f * 0.5f };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (input.keyboard_.GetTrigger(DIK_NUMPAD9)) {
			float PI = 3.14f;
			// 蜿榊ｯｾ譁ｹ蜷・
			float thetaOpposite = PI - targetRotate_.y;
			float phiOpposite = targetRotate_.z + PI;

			targetRotate_ = { distance, thetaOpposite, phiOpposite };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (input.keyboard_.GetTrigger(DIK_NUMPAD3)) {
			targetRotate_ = { distance, 3.14f * 0.5f,0.0f };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (cameraMoveT_ < 1.0f) {
			cameraMoveT_ += 0.1f;
			Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(targetRotate_);
			cameraTransform.translate = Vector3::Slerp(startPos_, sphericalToCartesian + anchorPoint_, cameraMoveT_);

			// LookAt縺ｮ譁ｹ蜷代・繧ｯ繝医Ν縺後ぞ繝ｭ縺ｫ縺ｪ繧峨↑縺・ｈ縺・↓
			if ((anchorPoint_ - Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix())).Length() > 0.001f) {
				cameraTransform.rotate = -Vector3::LookAt(anchorPoint_, Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix()));
			}
			return;
		}

		// 繝帙う繝ｼ繝ｫ縺ｧ繧ｺ繝ｼ繝繧､繝ｳ繝ｻ繧｢繧ｦ繝・
		if (input.mouse_.wheelDir_ != 0.0f) {
			Vector3 cartesianTemp = cameraTransform.translate - anchorPoint_;
			Vector3 sphericalTemp = Vector3::CartesianToSpherical(cartesianTemp);

			// 繝槭え繧ｹ縺ｮX遘ｻ蜍輔〒ﾏ・ｼ育ｵ悟ｺｦ, Yaw・峨〆遘ｻ蜍輔〒ﾎｸ・育ｷｯ蠎ｦ, Pitch・峨ｒ蝗櫁ｻ｢
			sphericalTemp.x += -input.mouse_.wheelDir_ * 0.01f;

			Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(sphericalTemp);
			cameraTransform.translate = sphericalToCartesian + anchorPoint_;

			cameraTransform.rotate = -Vector3::LookAt(anchorPoint_, Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix()));
		}

		if (input.mouse_.GetPress(2)) {

			if (input.keyboard_.GetPress(DIK_LSHIFT)) {
				Vector2 mouseMove = input.mouse_.deltaMouse_ * mouseSensitivity_ * 0.1f;
				Vector3 mouseMove3 = { -mouseMove.x,mouseMove.y,0.0f };
				Vector3 move = Vector3::Transform(mouseMove3, Matrix4x4::MakeRotateXYZMatrix(cameraTransform.rotate));
				cameraTransform.translate += move;
				anchorPoint_ += move;

			} else {
				Vector3 cartesianTemp = cameraTransform.translate - anchorPoint_;
				Vector3 sphericalTemp = Vector3::CartesianToSpherical(cartesianTemp);

				// 繝槭え繧ｹ縺ｮX遘ｻ蜍輔〒ﾏ・ｼ育ｵ悟ｺｦ, Yaw・峨〆遘ｻ蜍輔〒ﾎｸ・育ｷｯ蠎ｦ, Pitch・峨ｒ蝗櫁ｻ｢
				sphericalTemp.z += -input.mouse_.deltaMouse_.x * mouseSensitivity_ * 0.005f; // ﾏ・ 蟾ｦ蜿ｳ・域─蠎ｦ繧剃ｸ九￡繧具ｼ・
				sphericalTemp.y += -input.mouse_.deltaMouse_.y * mouseSensitivity_ * 0.005f; // ﾎｸ: 荳贋ｸ具ｼ域─蠎ｦ繧剃ｸ九￡繧具ｼ・

				// ﾎｸ・育ｷｯ蠎ｦ・峨・繧ｯ繝ｩ繝ｳ繝・
				const float epsilon = 0.01f;
				const float minTheta = epsilon;
				const float maxTheta = static_cast<float>(3.14159f) - epsilon;
				sphericalTemp.y = std::clamp(sphericalTemp.y, minTheta, maxTheta);

				Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(sphericalTemp);
				cameraTransform.translate = sphericalToCartesian + anchorPoint_;

				// LookAt縺ｮ譁ｹ蜷代・繧ｯ繝医Ν縺後ぞ繝ｭ縺ｫ縺ｪ繧峨↑縺・ｈ縺・↓
				if ((anchorPoint_ - Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix())).Length() > 0.001f) {
					cameraTransform.rotate = -Vector3::LookAt(anchorPoint_, Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix()));
				}
			}
		}
	}
#endif // _DEBUG
}

void SceneView::UpdateGizmo() {
#ifdef _DEBUG
	// ImGuizmo縺ｮ繧ｻ繝・ヨ繧｢繝・・
	bool is2D = AssetManager::GetInstance()->GetEntityManager()->HasComponent<SpriteData>(selectEntityId_);
	ImGuizmo::SetOrthographic(is2D);
	ImGuizmo::SetDrawlist();

	if (!AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(selectEntityId_)) {
		return;
	}
	Transform& transform = AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(selectEntityId_);

	Camera& camera = CameraManager::GetInstance()->GetCamera(0);
	Matrix4x4 view;
	Matrix4x4 proj;
	if (is2D) {
		view = Matrix4x4::MakeIndentity4x4();
		proj = camera.GetOrthographicMatrix();
	} else {
		view = camera.GetViewMatrix();
		proj = camera.GetPerspectiveMatrix();
	}

	static ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_T)) currentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) currentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) currentGizmoOperation = ImGuizmo::SCALE;

	ImGui::PushID("GizmoOperationMenu");
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) {
		ImGui::OpenPopup("GizmoOperationPopup");
	}
	if (ImGui::BeginPopup("GizmoOperationPopup")) {
		ImGui::Text("Gizmo Operation");
		ImGui::Separator();
		if (ImGui::MenuItem("Translate", nullptr, currentGizmoOperation == ImGuizmo::TRANSLATE)) {
			currentGizmoOperation = ImGuizmo::TRANSLATE;
		}
		if (ImGui::MenuItem("Rotate", nullptr, currentGizmoOperation == ImGuizmo::ROTATE)) {
			currentGizmoOperation = ImGuizmo::ROTATE;
		}
		if (ImGui::MenuItem("Scale", nullptr, currentGizmoOperation == ImGuizmo::SCALE)) {
			currentGizmoOperation = ImGuizmo::SCALE;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Toggle Gizmo", nullptr, isDrawGizmo_)) {
			isDrawGizmo_ = !isDrawGizmo_;
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (!isDrawGizmo_) {
		return;
	}

	// 繧ｮ繧ｺ繝｢逕ｨ陦悟・
	Matrix4x4 matrix = Matrix4x4::MakeAffineMatrix(
		transform.scale,
		transform.rotate,
		transform.translate
	);
	float* matrixPtr = &matrix.m[0][0];

	// 繧ｮ繧ｺ繝｢謠冗判繝ｻ謫堺ｽ・
	ImGuizmo::Manipulate(
		&view.m[0][0], &proj.m[0][0],
		currentGizmoOperation, ImGuizmo::LOCAL, matrixPtr
	);

	// 繧ｮ繧ｺ繝｢謫堺ｽ應ｸｭ縺ｮ縺ｿTransform縺ｫ蜿肴丐
	if (ImGuizmo::IsUsing()) {
		Matrix4x4 newMat;
		std::memcpy(&newMat.m[0][0], matrixPtr, sizeof(float) * 16);
		transform.FromMatrix(newMat);
	}
	// 繧ｮ繧ｺ繝｢謫堺ｽ懊＠縺ｦ縺・↑縺・→縺阪・Transform縺ｮ蛟､縺ｧ陦悟・繧貞・逕滓・・・atrix縺ｯ豈弱ヵ繝ｬ繝ｼ繝蜀咲函謌舌＆繧後ｋ縺ｮ縺ｧOK・・
#endif
}
