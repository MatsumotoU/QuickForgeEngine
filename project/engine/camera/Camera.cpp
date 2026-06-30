#include "Camera.h"
using namespace QFE::CAMERA;

void Camera::Initialize(float left, float right, float top, float bottom, float nearZ, float farZ, float fov) {
	// カメラのパラメータを設定
	fovY_ = fov;
	aspectRatio_ = (right - left) / (top - bottom);
	nearZ_ = nearZ;
	farZ_ = farZ;
	// 画面の左、右、上、下の座標を設定
	left_ = left;
	right_ = right;
	top_ = top;
	bottom_ = bottom;

	// 透視投影行列と平行投影行列を作成
	CreatePerspectiveMatrix(fovY_, aspectRatio_, nearZ_, farZ_);
	CreateOrthographicMatrix(left_, right_, top_, bottom_, nearZ_, farZ_);
}

void Camera::CreatePerspectiveMatrix(float fovY, float aspectRatio, float nearZ, float farZ) {
	// 透視投影行列を作成
	MATH::Matrix4x4 perspectiveMatrix = MATH::Matrix4x4::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ);
}

void Camera::CreateOrthographicMatrix(float left, float right, float top, float bottom, float nearZ, float farZ) {
	// 平行投影行列を作成
	MATH::Matrix4x4 orthographicMatrix = MATH::Matrix4x4::MakeOrthographicMatrix(left, right, top, bottom, nearZ, farZ);
}

QFE::MATH::Matrix4x4 Camera::GetViewProjectionMatrix(const MATH::Transform& cameraTranform, CameraType type) const {
	// カメラのワールド行列を作成
	MATH::Matrix4x4 cameraWorldMatrix = MATH::Matrix4x4::MakeAffineMatrix(cameraTranform);

	// ビュー行列を作成
	MATH::Matrix4x4 cameraViewMatrix = cameraWorldMatrix.Inverse();

	// 投影行列を作成
	MATH::Matrix4x4 projectionMatrix;
	if (type == CameraType::Perspective) {
		projectionMatrix = MATH::Matrix4x4::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearZ_, farZ_);
	} else {
		projectionMatrix = MATH::Matrix4x4::MakeOrthographicMatrix(left_, right_, top_, bottom_, nearZ_, farZ_);
	}

	// ビュー行列と投影行列を掛け合わせた行列を作成
	MATH::Matrix4x4 viewProjectionMatrix = MATH::Matrix4x4::Multiply(cameraViewMatrix, projectionMatrix);

	return viewProjectionMatrix;
}