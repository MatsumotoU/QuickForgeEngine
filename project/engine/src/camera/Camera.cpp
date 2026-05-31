#include "engine/include/camera/Camera.h"
#include <cassert>
#include "engine/include/core/EngineGlobalValue.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#endif // QFE_OPTIMIZE_OFF

#include "engine/include/assets/AssetManager.h" 
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/camera/Data/CameraData.h"
#include "engine/include/core/Math/TransformComponent.h"

using namespace QFE;

void Camera::Initialize() {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	bindEntityId_ = entityManager->CreateEntity();
	entityManager->EmplaceComponent<TransformComponent>(bindEntityId_);
	entityManager->EmplaceComponent<SceneObjectData>(bindEntityId_);
	entityManager->EmplaceComponent<CameraData>(bindEntityId_);

	assert(entityManager->HasComponent<TransformComponent>(bindEntityId_) && "Camera entity must have EulerTransform component.");
	assert(entityManager->HasComponent<SceneObjectData>(bindEntityId_) && "Camera entity must have SceneObjectData component.");
	assert(entityManager->HasComponent<CameraData>(bindEntityId_) && "Camera entity must have CameraData component.");
	EulerTransform& transform = entityManager->GetComponent<TransformComponent>(bindEntityId_).transform;
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
	projectionMatrix_ = Matrix4x4::MakeIndentity4x4();
	orthographicMatrix_ = Matrix4x4::MakeIndentity4x4();
	viewProjectionMatrix_ = Matrix4x4::MakeIndentity4x4();
	viewOrthographicMatrix_ = Matrix4x4::MakeIndentity4x4();
}

void Camera::Update() {
	viewMatrix_ = GetViewMatrix();
	projectionMatrix_ = GetPerspectiveMatrix();
	orthographicMatrix_ = GetOrthographicMatrix();
	viewProjectionMatrix_ = Matrix4x4::Multiply(viewMatrix_, projectionMatrix_);
	viewOrthographicMatrix_ = Matrix4x4::Multiply(Matrix4x4::MakeIndentity4x4(), orthographicMatrix_);
}

bool Camera::CheckVisible(const Matrix4x4& world) const {
	// 繝ｯ繝ｼ繝ｫ繝牙ｺｧ讓吶・蜴溽せ繧貞叙蠕・
	Vector4 pos4(0.0f, 0.0f, 0.0f, 1.0f);
	Vector4 worldPos = Vector4::EulerTransform(pos4, world);

	// 繧ｯ繝ｪ繝・・遨ｺ髢薙∈螟画鋤
	Vector4 clipPos = Vector4::EulerTransform(worldPos, viewProjectionMatrix_);

	// w縺ｧ蜑ｲ縺｣縺ｦNDC縺ｸ
	if (clipPos.w == 0.0f) return false;
	Vector3 ndcPos = { clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w };

	// NDC遽・峇蜀・°蛻､螳夲ｼ・irectX: z縺ｯ0・・, OpenGL: z縺ｯ-1・・・・
	return
		ndcPos.x >= -1.0f && ndcPos.x <= 1.0f &&
		ndcPos.y >= -1.0f && ndcPos.y <= 1.0f &&
		ndcPos.z >= 0.0f && ndcPos.z <= 1.0f;
}

uint32_t Camera::GetBindEntityId() const {
	return bindEntityId_;
}

Matrix4x4 Camera::GetViewMatrix() const {
	return Matrix4x4::Inverse(GetWorldMatrix());
}

Matrix4x4 Camera::GetPerspectiveMatrix() const {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<CameraData>(bindEntityId_)) {
		return Matrix4x4::MakeIndentity4x4();
	}
	CameraData& cameraData = entityManager->GetComponent<CameraData>(bindEntityId_);

	float aspect = static_cast<float>(QFE::EngineGlobalValue::windowWidth) / static_cast<float>(QFE::EngineGlobalValue::windowHeight);
	return Matrix4x4::MakePerspectiveFovMatrix(cameraData.fovY_, aspect, cameraData.nearZ_, cameraData.farZ_);
}

Matrix4x4 Camera::GetOrthographicMatrix() const {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
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
		return Matrix4x4::Multiply(worldMatrix, viewProjectionMatrix_);
		break;
	case CameraType::Orthographic:
		return Matrix4x4::Multiply(worldMatrix, viewOrthographicMatrix_);
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
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<TransformComponent>(bindEntityId_)) {
		return Matrix4x4::MakeIndentity4x4();
	}
	EulerTransform& transform = entityManager->GetComponent<TransformComponent>(bindEntityId_).transform;

	return Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
}

Vector3 Camera::GetPosition() const
{
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	EulerTransform& transform = entityManager->GetComponent<TransformComponent>(bindEntityId_).transform;
	return transform.translate;
}
