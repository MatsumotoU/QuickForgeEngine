#include "Camera.h"
#include <cassert>
#include "Core/EngineGlobalValue.h"

void Camera::Initialize() {
	fovY_ = 0.45f;
	nearZ_ = 0.1f;
	farZ_ = 1000.0f;
	cameraType = CameraType::Perspective;
	viewMatrix_ = Matrix4x4::MakeIndentity4x4();
}

void Camera::Update() {
	viewMatrix_ = GetViewMatrix();
}

Matrix4x4 Camera::GetViewMatrix() const {
	return Matrix4x4::Inverse(GetWorldMatrix());
}

Matrix4x4 Camera::GetPerspectiveMatrix() const {
	float aspect = static_cast<float>(QFE::EngineGlobalValue::windowWidth) / static_cast<float>(QFE::EngineGlobalValue::windowHeight);
	return Matrix4x4::MakePerspectiveFovMatrix(fovY_, aspect, nearZ_, farZ_);
}

Matrix4x4 Camera::GetOrthographicMatrix() const {
	return Matrix4x4::MakeOrthographicMatrix(
		0.0f, 0.0f,
		static_cast<float>(QFE::EngineGlobalValue::windowWidth),
		static_cast<float>(QFE::EngineGlobalValue::windowHeight), nearZ_, farZ_);
}

Matrix4x4 Camera::GetWorldViewProjectionMatrix(const Matrix4x4& worldMatrix) const {
	switch (cameraType)
	{
	case CameraType::Perspective:
		return Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix_, GetPerspectiveMatrix()));
		break;
	case CameraType::Orthographic:
		return Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix_, GetOrthographicMatrix()));
		break;
	default:
		assert(false && "Unknown Camera Type.");
		break;
	}
	return Matrix4x4();
}

Matrix4x4 Camera::GetWorldMatrix() const {
	return Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate); 
}
