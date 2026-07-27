#include "SceneFrameWork.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "components/TransformHierarchy.h"
#include "graphics/D3D12GraphicEngine.h"

#include "framework/graphic/D3D12GraphicFrameWork.h"

uint32_t QFE::FRAMEWORK::CreateEntity(QFE::SCENE::SceneManager& sceneManager, const std::string& name) {
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	uint32_t entity = entityManager.CreateEntity();
	entityManager.EmplaceComponent<QFE::SCENE::ObjectInfoComponent>(entity);
	entityManager.EmplaceComponent<QFE::SCENE::TransformComponent>(entity);
	entityManager.GetComponent<QFE::SCENE::ObjectInfoComponent>(entity).name = name;
	return entity;
}

uint32_t QFE::FRAMEWORK::CreateEntityWithMaterial(QFE::SCENE::SceneManager& sceneManager, const std::string& name, const QFE::MATH::Vector4& albedoColor) {
	uint32_t entity = CreateEntity(sceneManager, name);
	EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	entityManager.EmplaceComponent<QFE::SCENE::MaterialComponent>(entity);
	entityManager.GetComponent<QFE::SCENE::MaterialComponent>(entity).albedoColor = albedoColor;
	return entity;
}

QFE::MATH::Matrix4x4 QFE::FRAMEWORK::UpdateMainCamera(QFE::SCENE::SceneManager& sceneManager) {
	QFE::MATH::Matrix4x4 viewProjection = QFE::MATH::Matrix4x4::MakeIdentity4x4();
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	entityManager.Each<QFE::SCENE::CameraComponent>(
		[&](uint32_t entityId, QFE::SCENE::CameraComponent& camera) {
			if (!camera.isMainCamera || !entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				return;
			}
			camera.viewMatrix = QFE::SCENE::GetWorldMatrix(entityManager, entityId).Inverse();
			const float height = camera.top_ - camera.bottom_;
			camera.aspectRatio_ = height != 0.0f
				? fabsf((camera.right_ - camera.left_) / height)
				: 1.0f;
			camera.projectionMatrix = QFE::MATH::Matrix4x4::MakePerspectiveFovMatrix(
				camera.fovY_, camera.aspectRatio_, camera.nearZ_, camera.farZ_);
			viewProjection = QFE::MATH::Matrix4x4::Multiply(camera.viewMatrix, camera.projectionMatrix);
		});
	return viewProjection;
}

void QFE::FRAMEWORK::DrawSceneModels(
	QFE::SCENE::SceneManager& sceneManager,
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
	QFE::GRAPHIC::PSOHandle psoHandle,
	QFE::GRAPHIC::ViewPortHandle viewportHandle,
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle,
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
	const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes) {
	if (!graphicEngine) return;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	entityManager.Each<QFE::SCENE::ModelRenderComponent>(
		[&](uint32_t, QFE::SCENE::ModelRenderComponent& model) {
			if (!model.canRender) return;
			const std::vector<QFE::GRAPHIC::DirectXResourceHandle> rootResources = {
				static_cast<QFE::GRAPHIC::DirectXResourceHandle>(model.transformMatrixBufferHandle),
				static_cast<QFE::GRAPHIC::DirectXResourceHandle>(model.materialResourceHandle),
				static_cast<QFE::GRAPHIC::DirectXResourceHandle>(model.textureResourceHandle)
			};
			QFE::FRAMEWORK::DrawGraphicPSO(
				graphicEngine, psoHandle, viewportHandle, scissorRectHandle,
				static_cast<QFE::GRAPHIC::DirectXResourceHandle>(model.vertexResourceHandle),
				static_cast<QFE::GRAPHIC::DirectXResourceHandle>(model.indexResourceHandle),
				rootResources, renderTargets, rootParameterTypes);
		});
}
