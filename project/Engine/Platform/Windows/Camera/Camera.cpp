#include "Camera.h"
#include <cassert>
#include "Core/EngineGlobalValue.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#endif // _DEBUG

#include "Assets/AssetManager.h" 
#include "Core/Entity/EntityManager.h"
#include "Scene/Data/SceneObjectData.h"
#include "Data/CameraData.h"

void Camera::Initialize() {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	bindEntityId_ = entityManager->CreateEntity();
	entityManager->EmplaceComponent<Transform>(bindEntityId_);
	entityManager->EmplaceComponent<SceneObjectData>(bindEntityId_);
	entityManager->EmplaceComponent<CameraData>(bindEntityId_);

	assert(entityManager->HasComponent<Transform>(bindEntityId_) && "Camera entity must have Transform component.");
	assert(entityManager->HasComponent<SceneObjectData>(bindEntityId_) && "Camera entity must have SceneObjectData component.");
	assert(entityManager->HasComponent<CameraData>(bindEntityId_) && "Camera entity must have CameraData component.");
	Transform& transform = entityManager->GetComponent<Transform>(bindEntityId_);
	transform.scale = { 1.0f,1.0f,1.0f };
	transform.rotate = { 0.1f,0.0f,0.0f };
	transform.translate = { 0.0f,1.5f,-10.0f };
	SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(bindEntityId_);
	sceneObjectData.name = "Camera";
	sceneObjectData.tag = "camera";
	CameraData& cameraData = entityManager->GetComponent<CameraData>(bindEntityId_);
	cameraData.fovY_ = 0.45f;
	cameraData.nearZ_ = 0.1f;
	cameraData.farZ_ = 100.0f;

	viewMatrix_ = Matrix4x4::MakeIndentity4x4();
}

void Camera::Update() {
	viewMatrix_ = GetViewMatrix();
}

uint32_t Camera::GetBindEntityId() const {
	return bindEntityId_;
}

Matrix4x4 Camera::GetViewMatrix() const {
	return Matrix4x4::Inverse(GetWorldMatrix());
}

Matrix4x4 Camera::GetPerspectiveMatrix() const {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<CameraData>(bindEntityId_)) {
		return Matrix4x4::MakeIndentity4x4();
	}
	CameraData& cameraData = entityManager->GetComponent<CameraData>(bindEntityId_);

	float aspect = static_cast<float>(QFE::EngineGlobalValue::windowWidth) / static_cast<float>(QFE::EngineGlobalValue::windowHeight);
	return Matrix4x4::MakePerspectiveFovMatrix(cameraData.fovY_, aspect, cameraData.nearZ_, cameraData.farZ_);
}

Matrix4x4 Camera::GetOrthographicMatrix() const {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<CameraData>(bindEntityId_)) {
		return Matrix4x4::MakeIndentity4x4();
	}
	CameraData& cameraData = entityManager->GetComponent<CameraData>(bindEntityId_);

	return Matrix4x4::MakeOrthographicMatrix(
		0.0f, 0.0f,
		static_cast<float>(QFE::EngineGlobalValue::windowWidth),
		static_cast<float>(QFE::EngineGlobalValue::windowHeight), 0.01f, cameraData.farZ_);
}

Matrix4x4 Camera::GetWorldViewProjectionMatrix(const Matrix4x4& worldMatrix, CameraType type) const {
	switch (type)
	{
	case CameraType::Perspective:
		return Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix_, GetPerspectiveMatrix()));
		break;
	case CameraType::Orthographic:
		return Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(Matrix4x4::MakeIndentity4x4(), GetOrthographicMatrix()));
		break;
	default:
		assert(false && "Unknown Camera Type.");
		break;
	}
	return Matrix4x4();
}

Matrix4x4 Camera::GetWorldViewProjectionMatrixOrthographic(const Matrix4x4& worldMatrix) const {
	return Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(Matrix4x4::MakeIndentity4x4(), GetOrthographicMatrix()));
}

Matrix4x4 Camera::GetWorldMatrix() const {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<Transform>(bindEntityId_)) {
		return Matrix4x4::MakeIndentity4x4();
	}
	Transform& transform = entityManager->GetComponent<Transform>(bindEntityId_);

	return Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
}

#ifdef _DEBUG
void Camera::DrawImgui() {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	assert(entityManager->HasComponent<Transform>(bindEntityId_) && "Camera entity must have Transform component.");
	assert(entityManager->HasComponent<CameraData>(bindEntityId_) && "Camera entity must have CameraData component.");
	CameraData& cameraData = entityManager->GetComponent<CameraData>(bindEntityId_);
	Transform& transform = entityManager->GetComponent<Transform>(bindEntityId_);

	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::Text("Camera Type");
		const char* items[] = { "Perspective","Orthographic" };
		static int item_current = (cameraType == CameraType::Perspective) ? 0 : 1;
		if (ImGui::Combo("Type", &item_current, items, IM_ARRAYSIZE(items))) {
			cameraType = (item_current == 0) ? CameraType::Perspective : CameraType::Orthographic;
		}
		ImGui::SliderFloat("FovY", &cameraData.fovY_, 0.1f, 3.0f);
		ImGui::SliderFloat("NearZ", &cameraData.nearZ_, 0.01f, 10.0f);
		ImGui::SliderFloat("FarZ", &cameraData.farZ_, 10.0f, 1000.0f);
		ImGui::DragFloat3("Position", &transform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.1f);
	}
}
#endif // _DEBUG