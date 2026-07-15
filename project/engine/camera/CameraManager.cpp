#include "CameraManager.h"

using namespace QFE::CAMERA;

void CameraManager::Initialize() {
	cameras_.clear();
}

CameraHandle CameraManager::CreateCamera(float left, float right, float top, float bottom, float nearZ, float farZ, float fov) {
	Camera camera;
	camera.Initialize(left, right, top, bottom, nearZ, farZ, fov);
	return static_cast<CameraHandle>(cameras_.push_back(camera));
}

QFE::MATH::Matrix4x4 CameraManager::GetViewProjectionMatrix(CameraHandle handle, const MATH::Transform& cameraTranform, CameraType type) {
	return cameras_.Get(static_cast<uint32_t>(handle))->GetViewProjectionMatrix(cameraTranform, type);
}

QFE::MATH::Matrix4x4 CameraManager::GetWVPMatrix(CameraHandle handle, const MATH::Matrix4x4& worldMatrix, const MATH::Transform& cameraTranform, CameraType type) {
	return QFE::MATH::Matrix4x4::Multiply(worldMatrix, GetViewProjectionMatrix(handle, cameraTranform, type));
}
