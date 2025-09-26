#include "SceneView.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"
#include "Assets/Camera/CameraManager.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Input/DirectInput/DirectInputManager.h"

#include <cmath>
#include <algorithm>

SceneView::SceneView() {
	name_ = "Scene View";
	isActiveCamera_ = true;

	anchorPoint_ = { 0.0f,0.0f,0.0f };
	mouseSensitivity_ = 0.5f;
}

void SceneView::Initialize() {
	isActive_ = true;
}

void SceneView::Update() {
#ifdef _DEBUG
	Camera& camera = CameraManager::GetInstance()->GetCamera(0);
	if (isActiveCamera_) {
		DirectInputManager* input = DirectInputManager::GetInstance();
		
		if (input->mouse_.wheelDir_ != 0.0f) {
			Vector3 cartesianTemp = camera.transform_.translate - anchorPoint_;
			Vector3 sphericalTemp = Vector3::CartesianToSpherical(cartesianTemp);

			// マウスのX移動でφ（経度, Yaw）、Y移動でθ（緯度, Pitch）を回転
			sphericalTemp.x += -input->mouse_.wheelDir_ * 0.01f;

			Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(sphericalTemp);
			camera.transform_.translate = sphericalToCartesian + anchorPoint_;

			camera.transform_.rotate = -Vector3::LookAt(anchorPoint_, Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix()));
		}

		if (input->mouse_.GetPress(2)) {

			if (input->keyboard_.GetPress(DIK_LSHIFT)) {
				Vector2 mouseMove = input->mouse_.deltaMouse_ * mouseSensitivity_ * 0.1f;
				Vector3 mouseMove3 = { -mouseMove.x,mouseMove.y,0.0f };
				Vector3 move = Vector3::Transform(mouseMove3, Matrix4x4::MakeRotateXYZMatrix(camera.transform_.rotate));
				camera.transform_.translate += move;
				anchorPoint_ += move;

			} else {
				Vector3 cartesianTemp = camera.transform_.translate - anchorPoint_;
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
				camera.transform_.translate = sphericalToCartesian + anchorPoint_;

				// LookAtの方向ベクトルがゼロにならないように
				if ((anchorPoint_ - Vector3::Transform({0.0f,0.0f,0.0f}, camera.GetWorldMatrix())).Length() > 0.001f) {
					camera.transform_.rotate = -Vector3::LookAt(anchorPoint_, Vector3::Transform({ 0.0f,0.0f,0.0f }, camera.GetWorldMatrix()));
				}
			}
		}

	}
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

    ImVec2 availSize = ImGui::GetContentRegionAvail();
    float targetAspect = 1280.0f / 720.0f;
    ImVec2 imageSize;
    if (availSize.x / availSize.y > targetAspect) {
        imageSize.y = availSize.y;
        imageSize.x = availSize.y * targetAspect;
    } else {
        imageSize.x = availSize.x;
        imageSize.y = availSize.x / targetAspect;
    }
    ImVec2 centerPos = ImVec2(
        (availSize.x - imageSize.x) * 0.5f,
        (availSize.y - imageSize.y) * 0.5f
    );
    ImGui::SetCursorPos(centerPos);
    ImGui::Image((void*)handle.gpuHandle_.ptr, imageSize);

    ImGui::End();
#endif // _DEBUG
}
