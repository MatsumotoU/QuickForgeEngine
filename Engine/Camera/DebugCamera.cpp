#include "DebugCamera.h"
#include "../Input/DirectInput/DirectInputManager.h"
#include "../Base/DirectX/ImGuiManager.h"

#ifdef _DEBUG
void DebugCamera::Initialize(WinApp* win, DirectInputManager* input) {
	camera_.Initialize(win);
	input_ = input;
	mouseSensitivity_ = 0.5f;

	transform_ = {};
	transform_.scale = { 1.0f,1.0f,1.0f };
}

void DebugCamera::Update() {
	camera_.Update();

	if (input_->mouse_.GetPress(1)) {
		transform_.translate.x -= input_->mouse_.mouseMoveDir_.x * mouseSensitivity_ * 0.1f;
		transform_.translate.y += input_->mouse_.mouseMoveDir_.y * mouseSensitivity_ * 0.1f;
	}

	if (input_->mouse_.GetPress(2)) {
		transform_.rotate.x += input_->mouse_.mouseMoveDir_.y * mouseSensitivity_ * 0.1f;
		transform_.rotate.y += input_->mouse_.mouseMoveDir_.x * mouseSensitivity_ * 0.1f;
	}

	transform_.scale.x -= input_->mouse_.wheelDir_ * 0.001f;
	transform_.scale.y -= input_->mouse_.wheelDir_ * 0.001f;
	transform_.scale.z -= input_->mouse_.wheelDir_ * 0.001f;

	Matrix4x4 scaleMatrix = Matrix4x4::MakeScaleMatrix(transform_.scale);
	camera_.affineMatrix_ = Matrix4x4::Multiply(camera_.affineMatrix_, scaleMatrix);
	Matrix4x4 translateMatrix = Matrix4x4::MakeTranslateMatrix(transform_.translate);
	camera_.affineMatrix_ = Matrix4x4::Multiply(camera_.affineMatrix_, translateMatrix);
	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateXYZMatrix(transform_.rotate);
	camera_.affineMatrix_ = Matrix4x4::Multiply(camera_.affineMatrix_, rotateMatrix);
}

void DebugCamera::DrawImGui() {
	ImGui::Begin("DebugCamera");
	ImGui::DragFloat3("CameraTranslate", &camera_.transform_.translate.x);
	ImGui::DragFloat3("CameraRotate", &camera_.transform_.rotate.x);
	ImGui::DragFloat3("CameraScale", &camera_.transform_.scale.x);

	if (ImGui::Button("ResetCamera")) {
		transform_ = {};
		transform_.scale = { 1.0f,1.0f,1.0f };
	}
	ImGui::End();
}
#endif // _DEBUG