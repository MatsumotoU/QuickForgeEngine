#include "DebugCamera.h"
#include "Base/EngineCore.h"
#include "../Input/DirectInput/DirectInputManager.h"
#include "../Base/DirectX/ImGuiManager.h"

#include <algorithm>

#ifdef _DEBUG

DebugCamera::DebugCamera(EngineCore* engineCore){
	anchorPointBillboard_ = std::make_unique<Billboard>(engineCore, &camera_,1.0f, 1.0f, 0);
}

DebugCamera::~DebugCamera() {
	engineCore_->GetLoopStopper()->RemoveNonStoppingFunc();
}

void DebugCamera::Initialize(EngineCore* engineCore) {
	camera_.Initialize(engineCore->GetWinApp());
	input_ = engineCore->GetInputManager();
	mouseSensitivity_ = 0.5f;

	transform_ = {};
	transform_.scale = { 1.0f,1.0f,1.0f };

	anchorPoint_ = {};

	engineCore_ = engineCore;
	anchorPointBillboard_->Init();
	anchorGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/anchor.png");

	isDrawAnchor_ = false;

	engineCore_->GetLoopStopper()->AddNonStoppingFunc(std::bind(&DebugCamera::Update, this));
}

void DebugCamera::Update() {

	if (input_->mouse_.wheelDir_ != 0.0f) {
		Vector3 cartesianTemp = camera_.transform_.translate - anchorPoint_;
		Vector3 sphericalTemp = Vector3::CartesianToSpherical(cartesianTemp);

		// マウスのX移動でφ（経度, Yaw）、Y移動でθ（緯度, Pitch）を回転

		sphericalTemp.x += -input_->mouse_.wheelDir_ * 0.01f;

		Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(sphericalTemp);
		camera_.transform_.translate = sphericalToCartesian + anchorPoint_;

		camera_.transform_.rotate = -Vector3::LookAt(anchorPoint_, camera_.GetWorldPos());
	}

	if (input_->mouse_.GetPress(2)) {

		if (input_->keyboard_.GetPress(DIK_LSHIFT)) {
			Vector2 mouseMove = input_->mouse_.deltaMouse_ * mouseSensitivity_ * 0.1f;
			Vector3 mouseMove3 = { -mouseMove.x,mouseMove.y,0.0f };
			Vector3 move = Vector3::Transform(mouseMove3, Matrix4x4::MakeRotateXYZMatrix(camera_.transform_.rotate));
			camera_.transform_.translate += move;
			anchorPoint_ += move;

		} else {
			Vector3 cartesianTemp = camera_.transform_.translate - anchorPoint_;
			Vector3 sphericalTemp = Vector3::CartesianToSpherical(cartesianTemp);

			// マウスのX移動でφ（経度, Yaw）、Y移動でθ（緯度, Pitch）を回転
			sphericalTemp.z += -input_->mouse_.deltaMouse_.x * mouseSensitivity_ * 0.005f; // φ: 左右（感度を下げる）
			sphericalTemp.y += -input_->mouse_.deltaMouse_.y * mouseSensitivity_ * 0.005f; // θ: 上下（感度を下げる）

			// θ（緯度）のクランプ
			const float epsilon = 0.01f;
			const float minTheta = epsilon;
			const float maxTheta = static_cast<float>(M_PI) - epsilon;
			sphericalTemp.y = std::clamp(sphericalTemp.y, minTheta, maxTheta);

			Vector3 sphericalToCartesian = Vector3::SphericalToCartesian(sphericalTemp);
			camera_.transform_.translate = sphericalToCartesian + anchorPoint_;

			// LookAtの方向ベクトルがゼロにならないように
			if ((anchorPoint_ - camera_.GetWorldPos()).Length() > 0.001f) {
				camera_.transform_.rotate = -Vector3::LookAt(anchorPoint_, camera_.GetWorldPos());
			}
		}
	}

	camera_.Update();
	anchorPointBillboard_->transform_.translate = anchorPoint_;
	anchorPointBillboard_->Update();
}

void DebugCamera::DrawImGui() {
	ImGui::Begin("DebugCamera");
	ImGui::DragFloat3("DebugTranslate", &transform_.translate.x);
	ImGui::DragFloat3("DebugRotate", &transform_.rotate.x);
	ImGui::DragFloat3("DebugScale", &transform_.scale.x);

	ImGui::DragFloat3("CameraTranslate", &camera_.transform_.translate.x);
	ImGui::DragFloat3("CameraRotate", &camera_.transform_.rotate.x);
	ImGui::DragFloat3("CameraScale", &camera_.transform_.scale.x);

	ImGui::DragFloat3("CameraAnchorPoint", &anchorPoint_.x);

	ImGui::Text("IsDrawAnchor: %s", isDrawAnchor_ ? "True" : "False");
	if (ImGui::Button("IsDrawAnchor")) {
		isDrawAnchor_ = !isDrawAnchor_;
	}

	if (ImGui::Button("ResetCamera")) {

		anchorPoint_ = {};

		camera_.transform_ = {};
		camera_.transform_.translate.z = -5.0f;
		camera_.transform_.scale = { 1.0f,1.0f,1.0f };

		transform_ = {};
		transform_.scale = { 1.0f,1.0f,1.0f };
	}
	ImGui::End();

	if (isDrawAnchor_) {
		anchorPointBillboard_->Draw();
	}

}
#endif // _DEBUG