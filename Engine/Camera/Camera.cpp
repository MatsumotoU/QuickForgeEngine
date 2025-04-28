#include "Camera.h"
#include <cassert>
#include "../Base/Windows/WinApp.h"

Camera::Camera() {
	transform_ = {};
	transform_.translate = { 0.0f,0.0f,-5.0f };
	transform_.scale = { 1.0f,1.0f,1.0f };
	affineMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Matrix4x4::MakeIndentity4x4();
	perspectiveMatrix_ = Matrix4x4::MakeIndentity4x4();
	orthographicMatrix_ = Matrix4x4::MakeIndentity4x4();

	
}

Camera::~Camera() {
}

void Camera::Initialize(WinApp* win) {
	perspectiveMatrix_ = Matrix4x4::MakePerspectiveFovMatrix(0.45f, static_cast<float>(win->kWindowWidth) / static_cast<float>(win->kWindowHeight), 0.1f, 100.0f);
	orthographicMatrix_ = Matrix4x4::MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(win->kWindowWidth), static_cast<float>(win->kWindowHeight), 0.0f, 100.0f);
}

void Camera::Update() {
	affineMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

Matrix4x4 Camera::MakeWorldViewProjectionMatrix(const Matrix4x4& worldMatrix, ViewState viewState) {
	Matrix4x4 result{};

	switch (viewState)
	{
	case CAMERA_VIEW_STATE_PERSPECTIVE:
		viewMatrix_ = Matrix4x4::Inverse(affineMatrix_);
		result = Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix_, perspectiveMatrix_));
		break;

	case CAMERA_VIEW_STATE_ORTHOGRAPHIC:
		viewMatrix_ = Matrix4x4::MakeIndentity4x4();
		result = Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix_, orthographicMatrix_));
		break;
	default:
		assert(false);
		break;
	}
	return result;
}
