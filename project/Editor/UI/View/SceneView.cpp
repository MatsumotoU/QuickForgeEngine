#include "SceneView.h"
#include "HierarchyView.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"
#include "Camera/CameraManager.h"
#include "Assets/AssetManager.h"
#include "Core/Entity/EntityManager.h" 
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Input/DirectInput/DirectInputManager.h"
#include "Renderer/GraphRenderer.h"
#include "Core/Math/MyMath.h"

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

	// フォーカス判定
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
		DirectInputManager* input = DirectInputManager::GetInstance();

		float distance = (cameraTransform.translate - anchorPoint_).Length();
		if (input->keyboard_.GetTrigger(DIK_NUMPAD7)) {
			targetRotate_ = { distance, 0.0f, 3.14f };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (input->keyboard_.GetTrigger(DIK_NUMPAD1)) {
			targetRotate_ = { distance, -3.14f * 0.5f,3.14f * 0.5f };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (input->keyboard_.GetTrigger(DIK_NUMPAD9)) {
			float PI = 3.14f;
			// 反対方向
			float thetaOpposite = PI - targetRotate_.y;
			float phiOpposite = targetRotate_.z + PI;

			targetRotate_ = { distance, thetaOpposite, phiOpposite };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (input->keyboard_.GetTrigger(DIK_NUMPAD3)) {
			targetRotate_ = { distance, 3.14f * 0.5f,0.0f };
			startPos_ = cameraTransform.translate;
			cameraMoveT_ = 0.0f;
		}
		if (cameraMoveT_ < 1.0f) {
			cameraMoveT_ += 0.1f;
			Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(targetRotate_);
			cameraTransform.translate = Vector3::Slerp(startPos_, sphericalToCartesian + anchorPoint_, cameraMoveT_);

			// LookAtの方向ベクトルがゼロにならないように
			if ((anchorPoint_ - Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix())).Length() > 0.001f) {
				cameraTransform.rotate = -Vector3::LookAt(anchorPoint_, Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix()));
			}
			return;
		}

		// ホイールでズームイン・アウト
		if (input->mouse_.wheelDir_ != 0.0f) {
			Vector3 cartesianTemp = cameraTransform.translate - anchorPoint_;
			Vector3 sphericalTemp = Vector3::CartesianToSpherical(cartesianTemp);

			// マウスのX移動でφ（経度, Yaw）、Y移動でθ（緯度, Pitch）を回転
			sphericalTemp.x += -input->mouse_.wheelDir_ * 0.01f;

			Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(sphericalTemp);
			cameraTransform.translate = sphericalToCartesian + anchorPoint_;

			cameraTransform.rotate = -Vector3::LookAt(anchorPoint_, Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix()));
		}

		if (input->mouse_.GetPress(2)) {

			if (input->keyboard_.GetPress(DIK_LSHIFT)) {
				Vector2 mouseMove = input->mouse_.deltaMouse_ * mouseSensitivity_ * 0.1f;
				Vector3 mouseMove3 = { -mouseMove.x,mouseMove.y,0.0f };
				Vector3 move = Vector3::Transform(mouseMove3, Matrix4x4::MakeRotateXYZMatrix(cameraTransform.rotate));
				cameraTransform.translate += move;
				anchorPoint_ += move;

			} else {
				Vector3 cartesianTemp = cameraTransform.translate - anchorPoint_;
				Vector3 sphericalTemp = Vector3::CartesianToSpherical(cartesianTemp);

				// マウスのX移動でφ（経度, Yaw）、Y移動でθ（緯度, Pitch）を回転
				sphericalTemp.z += -input->mouse_.deltaMouse_.x * mouseSensitivity_ * 0.005f; // φ: 左右（感度を下げる）
				sphericalTemp.y += -input->mouse_.deltaMouse_.y * mouseSensitivity_ * 0.005f; // θ: 上下（感度を下げる）

				// θ（緯度）のクランプ
				const float epsilon = 0.01f;
				const float minTheta = epsilon;
				const float maxTheta = static_cast<float>(3.14159f) - epsilon;
				sphericalTemp.y = std::clamp(sphericalTemp.y, minTheta, maxTheta);

				Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(sphericalTemp);
				cameraTransform.translate = sphericalToCartesian + anchorPoint_;

				// LookAtの方向ベクトルがゼロにならないように
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
	// ImGuizmoのセットアップ
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

	// ギズモ用行列
	Matrix4x4 matrix = Matrix4x4::MakeAffineMatrix(
		transform.scale,
		transform.rotate,
		transform.translate
	);
	float* matrixPtr = &matrix.m[0][0];

	// ギズモ描画・操作
	ImGuizmo::Manipulate(
		&view.m[0][0], &proj.m[0][0],
		currentGizmoOperation, ImGuizmo::LOCAL, matrixPtr
	);

	// ギズモ操作中のみTransformに反映
	if (ImGuizmo::IsUsing()) {
		Matrix4x4 newMat;
		std::memcpy(&newMat.m[0][0], matrixPtr, sizeof(float) * 16);
		transform.FromMatrix(newMat);
	}
	// ギズモ操作していないときはTransformの値で行列を再生成（matrixは毎フレーム再生成されるのでOK）
#endif
}