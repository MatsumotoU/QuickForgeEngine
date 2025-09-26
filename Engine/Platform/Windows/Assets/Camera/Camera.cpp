#include "Camera.h"
#include <cassert>
#include "Core/EngineGlobalValue.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#endif // _DEBUG

void Camera::Initialize() {
	fovY_ = 0.45f;
	nearZ_ = 0.1f;
	farZ_ = 100.0f;
	cameraType = CameraType::Perspective;
	viewMatrix_ = Matrix4x4::MakeIndentity4x4();

	transform_.rotate = { 0.1f,0.0f,0.0f };
	transform_.translate = { 0.0f,1.5f,-10.0f };
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
		static_cast<float>(QFE::EngineGlobalValue::windowHeight), 0.0f, farZ_);
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

#ifdef _DEBUG
void Camera::DrawImgui() {
	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::Text("Camera Type");
		const char* items[] = { "Perspective","Orthographic" };
		static int item_current = (cameraType == CameraType::Perspective) ? 0 : 1;
		if (ImGui::Combo("Type", &item_current, items, IM_ARRAYSIZE(items))) {
			cameraType = (item_current == 0) ? CameraType::Perspective : CameraType::Orthographic;
		}
		ImGui::SliderFloat("FovY", &fovY_, 0.1f, 3.0f);
		ImGui::SliderFloat("NearZ", &nearZ_, 0.01f, 10.0f);
		ImGui::SliderFloat("FarZ", &farZ_, 10.0f, 1000.0f);
		ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
		ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.1f);
	}
}
#endif // _DEBUG