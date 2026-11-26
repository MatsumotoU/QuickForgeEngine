#include "framework/scene/SceneCoreFramework.h"

// アプリケーションからの操作をここで受け、モジュールへの依存を吸収する。
void QFE::FRAMEWORK::InitializeScene(SCENE::SceneManager& scene) { scene.Initialize(); }
uint32_t QFE::FRAMEWORK::CreateSceneCamera(SCENE::SceneManager& scene) { return scene.CreateCameraEntity(); }
uint32_t QFE::FRAMEWORK::CreateSceneDirectionalLight(SCENE::SceneManager& scene) { return scene.CreateDirectionalLightEntity(); }
QFE::EntityManager& QFE::FRAMEWORK::GetSceneEntities(SCENE::SceneManager& scene) { return scene.GetCurrentSceneEntityManager(); }
const std::string& QFE::FRAMEWORK::GetScenePath(const SCENE::SceneManager& scene) { return scene.GetCurrentScenePath(); }
void QFE::FRAMEWORK::LoadScene(SCENE::SceneManager& scene, const std::string& path) { scene.LoadCurrentSceneFromJson(path); }
bool QFE::FRAMEWORK::LoadScene(SCENE::SceneManager& scene, const nlohmann::json& data) { return scene.LoadCurrentSceneFromJson(data); }
void QFE::FRAMEWORK::SaveScene(SCENE::SceneManager& scene, const std::string& path) { scene.SaveCurrentSceneToJson(path); }
nlohmann::json QFE::FRAMEWORK::ReadSceneJson(SCENE::SceneManager& scene, const std::string& path) { return scene.LoadCurrentSceneToJson(path); }
QFE::SCENE::SceneSnapshot QFE::FRAMEWORK::CaptureScene(SCENE::SceneManager& scene) { return scene.CreateSnapshot(); }
bool QFE::FRAMEWORK::RestoreScene(SCENE::SceneManager& scene, const SCENE::SceneSnapshot& snapshot) { return scene.RestoreSnapshot(snapshot); }
uint32_t QFE::FRAMEWORK::LoadSceneEntity(SCENE::SceneManager& scene, const std::string& path) { return scene.LoadEntityOnCurrentSceneFromJsonObject(path); }
uint32_t QFE::FRAMEWORK::AppendScene(SCENE::SceneManager& scene, const std::string& path, const QFE::MATH::Vector3& origin) { return scene.AppendScene(path, origin); }
#include "framework/scene/SceneCoreFramework.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "components/TransformHierarchy.h"

#include <algorithm>

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

QFE::MATH::Matrix4x4 QFE::FRAMEWORK::UpdateMainCamera(
	QFE::SCENE::SceneManager& sceneManager, QFE::MATH::Vector3* outCameraPosition) {
	QFE::MATH::Matrix4x4 viewProjection = QFE::MATH::Matrix4x4::MakeIdentity4x4();
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	entityManager.Each<QFE::SCENE::CameraComponent>(
		[&](uint32_t entityId, QFE::SCENE::CameraComponent& camera) {
			if (!camera.isMainCamera || !entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				return;
			}
			const QFE::MATH::Matrix4x4 cameraWorld = QFE::SCENE::GetWorldMatrix(entityManager, entityId);
			camera.viewMatrix = cameraWorld.Inverse();
			if (outCameraPosition != nullptr) {
				*outCameraPosition = { cameraWorld.m[3][0], cameraWorld.m[3][1], cameraWorld.m[3][2] };
			}
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
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
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
			QFE::GRAPHIC::DrawGraphicPSO(
				graphicEngine, psoHandle, viewportHandle, scissorRectHandle,
				static_cast<QFE::GRAPHIC::DirectXResourceHandle>(model.vertexResourceHandle),
				static_cast<QFE::GRAPHIC::DirectXResourceHandle>(model.indexResourceHandle),
				rootResources, renderTargets, rootParameterTypes);
		});
}

void QFE::FRAMEWORK::DrawSceneSprites(
	QFE::SCENE::SceneManager& sceneManager,
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	QFE::GRAPHIC::PSOHandle psoHandle,
	QFE::GRAPHIC::ViewPortHandle viewportHandle,
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle,
	QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle,
	QFE::GRAPHIC::RenderTargetHandle renderTarget) {
	if (!graphicEngine || vertexBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return;

	struct SpriteDrawItem {
		uint32_t entityId;
		QFE::SCENE::SpriteRenderComponent* sprite;
	};

	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	std::vector<SpriteDrawItem> drawItems;
	entityManager.Each<QFE::SCENE::SpriteRenderComponent>(
		[&](uint32_t entityId, QFE::SCENE::SpriteRenderComponent& sprite) {
			if (sprite.visible && sprite.canRender) {
				drawItems.push_back({ entityId, &sprite });
			}
		});

	std::stable_sort(drawItems.begin(), drawItems.end(),
		[](const SpriteDrawItem& lhs, const SpriteDrawItem& rhs) {
			if (lhs.sprite->drawOrder != rhs.sprite->drawOrder) {
				return lhs.sprite->drawOrder < rhs.sprite->drawOrder;
			}
			return lhs.entityId < rhs.entityId;
		});

	for (const SpriteDrawItem& item : drawItems) {
		const QFE::SCENE::SpriteRenderComponent& sprite = *item.sprite;
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle> rootResources = {
			static_cast<QFE::GRAPHIC::DirectXResourceHandle>(sprite.transformMatrixBufferHandle),
			static_cast<QFE::GRAPHIC::DirectXResourceHandle>(sprite.materialResourceHandle),
			static_cast<QFE::GRAPHIC::DirectXResourceHandle>(sprite.textureResourceHandle)
		};
		QFE::GRAPHIC::DrawGraphicPSO(
			graphicEngine, psoHandle, viewportHandle, scissorRectHandle,
			vertexBufferHandle, rootResources, { renderTarget });
	}
}

