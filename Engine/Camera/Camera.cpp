#include "Camera.h"
#include <cassert>
#include "../Base/Windows/WinApp.h"

Camera::Camera() {
	transform_ = {};
	transform_.translate = { 0.0f,0.0f,-5.0f };
	transform_.scale = { 1.0f,1.0f,1.0f };
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Matrix4x4::MakeIndentity4x4();
	perspectiveMatrix_ = Matrix4x4::MakeIndentity4x4();
	orthographicMatrix_ = Matrix4x4::MakeIndentity4x4();
	localPos_ = {};
}

Camera::~Camera() {
}

void Camera::Initialize(WinApp* win) {
	win_ = win;

	perspectiveMatrix_ = Matrix4x4::MakePerspectiveFovMatrix(0.45f, static_cast<float>(win->kWindowWidth) / static_cast<float>(win->kWindowHeight), 0.1f, 100.0f);
	orthographicMatrix_ = Matrix4x4::MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(win->kWindowWidth), static_cast<float>(win->kWindowHeight), 0.0f, 100.0f);

	viewport_.Inititalize(win);
	scissorrect_.Initialize(win);
}

void Camera::Update() {
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

Vector3 Camera::GetScreenPos(const Vector3& localPos, const Matrix4x4& worldMatrix) {
	Vector3 result{};
	result = Vector3::Transform(localPos, Matrix4x4::Multiply( MakeWorldViewProjectionMatrix(worldMatrix, ViewState::CAMERA_VIEW_STATE_PERSPECTIVE),GetViewPortMatrix()));
	return result;
}

Vector3 Camera::GetScreenToWorldPos(const Vector3& screen) {
	return Vector3::Transform(screen, Matrix4x4::Multiply(Matrix4x4::Multiply(viewMatrix_, perspectiveMatrix_), GetViewPortMatrix()).Inverse());
}

Vector3 Camera::GetWorldToScreenPos(const Vector3& world) {
	return Vector3::Transform(world, Matrix4x4::Multiply(Matrix4x4::Multiply(viewMatrix_, perspectiveMatrix_), GetViewPortMatrix()));
}

Matrix4x4 Camera::MakeWorldViewProjectionMatrix(const Matrix4x4& worldMatrix, ViewState viewState) {
	Matrix4x4 result{};

	switch (viewState)
	{
	case ViewState::CAMERA_VIEW_STATE_PERSPECTIVE:
		viewMatrix_ = Matrix4x4::Inverse(worldMatrix_);
		result = Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix_, perspectiveMatrix_));
		break;

	case ViewState::CAMERA_VIEW_STATE_ORTHOGRAPHIC:
		viewMatrix_ = Matrix4x4::MakeIndentity4x4();
		result = Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix_, orthographicMatrix_));
		break;
	default:
		assert(false);
		break;
	}
	return result;
}

Matrix4x4 Camera::GetViewPortMatrix() {
	return Matrix4x4::MakeViewportMatrix(0, 0, static_cast<float>(win_->kWindowWidth), static_cast<float>(win_->kWindowHeight), 0.0f, 1.0f);
}

Vector3 Camera::GetWorldPos() {
	return Vector3::Transform(localPos_,worldMatrix_);
}

Matrix4x4 Camera::GetWorldMatrix() const {
	return Matrix4x4::MakeAffineMatrix(transform_.scale,transform_.rotate,transform_.translate);
}

Matrix4x4 Camera::GetRotateMatrix() const {
	return Matrix4x4::MakeRotateXYZMatrix(transform_.rotate);
}
