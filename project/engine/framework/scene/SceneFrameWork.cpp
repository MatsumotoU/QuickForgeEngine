#include "SceneFrameWork.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "graphics/D3D12GraphicEngine.h"

#include "framework/graphic/D3D12GraphicFrameWork.h"

std::unique_ptr<QFE::SCENE::SceneManager> QFE::FRAMEWORK::CreateSceneManager() {
	std::unique_ptr<QFE::SCENE::SceneManager> sceneManager = std::make_unique<QFE::SCENE::SceneManager>();
	sceneManager->Initialize();
	return sceneManager;
}

bool QFE::FRAMEWORK::EndFrameSceneManager(QFE::SCENE::SceneManager* sceneManager) {
	sceneManager->EndFrame();
	return true;
}

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

bool QFE::FRAMEWORK::LoadSceneFromJson(QFE::SCENE::SceneManager* sceneManager, const std::string& filePath) {
	sceneManager->LoadCurrentSceneFromJson(filePath);
	return true;
}

bool QFE::FRAMEWORK::UpdateCurrentCamera(
	QFE::SCENE::SceneManager* sceneManager, QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine) {

	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();
	// カメラのビュー行列と投影行列を取得
	QFE::MATH::Matrix4x4 viewProj = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	entityManager.Each<QFE::SCENE::CameraComponent>([&](uint32_t entityId, QFE::SCENE::CameraComponent& cameraComp) {
		if (cameraComp.isMainCamera) {
			if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				QFE::MATH::EulerTransform& cameraTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
				cameraComp.viewMatrix = QFE::MATH::Matrix4x4::MakeAffineMatrix(cameraTransform).Inverse();
				if (cameraComp.top_ - cameraComp.bottom_ != 0.0f) {
					cameraComp.aspectRatio_ = fabsf((cameraComp.right_ - cameraComp.left_) / (cameraComp.top_ - cameraComp.bottom_));
				} else {
					cameraComp.aspectRatio_ = 1.0f; // デフォルトのアスペクト比
				}
				cameraComp.projectionMatrix = QFE::MATH::Matrix4x4::MakePerspectiveFovMatrix(
					cameraComp.fovY_, cameraComp.aspectRatio_, cameraComp.nearZ_, cameraComp.farZ_);

				viewProj = QFE::MATH::Matrix4x4::Multiply(cameraComp.viewMatrix, cameraComp.projectionMatrix);
			}
		}
		});
	return true;
}

bool QFE::FRAMEWORK::UpdateModelRenderComponents(
	QFE::SCENE::SceneManager* sceneManager, QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
	const QFE::MATH::Matrix4x4& viewProj) {
	
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();

	// 各エンティティのModelRenderComponentを更新
	std::vector<std::pair<QFE::GRAPHIC::BLASHandle, QFE::MATH::Matrix4x4>> raytracingInstances;
	entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
		modelRenderComp.canRender = false;
		// TransformComponentを取得して、EulerTransformを更新する
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId) == false) {
			modelRenderComp.renderErrorMessage = "Missing TransformComponent for entity: " + std::to_string(entityId);
			return;
		}
		QFE::MATH::EulerTransform objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;

		if (entityManager.HasComponent < QFE::SCENE::AnimationComponent>(entityId)) {
			QFE::SCENE::AnimationComponent& animationComp = entityManager.GetComponent<QFE::SCENE::AnimationComponent>(entityId);
			objTransform.translate += animationComp.transform.translate;
			objTransform.rotate += animationComp.transform.rotate;
			objTransform.scale += animationComp.transform.scale;
		}

		QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetDirectXResourceAllocator();
		QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
			resourceAllocator->AllocateConstantBuffer<TransformationMatrix>();
		QFE::FRAMEWORK::UpdateObject3dWVPMatrix(graphicEngine, transformMatrixBufferHandle, objTransform, viewProj);
		modelRenderComp.transformMatrixBufferHandle =
			static_cast<uint32_t>(transformMatrixBufferHandle);

		// マテリアルの更新
		if (entityManager.HasComponent<QFE::SCENE::MaterialComponent>(entityId) == false) {
			modelRenderComp.renderErrorMessage = "Missing MaterialComponent for entity: " + std::to_string(entityId);
			return;
		}
		QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle =
			resourceAllocator->AllocateConstantBuffer<Material>();
		Material* materialData = graphicEngine->GetConstantBufferData<Material>(materialBufferHandle);
		QFE::SCENE::MaterialComponent& materialComp = entityManager.GetComponent<QFE::SCENE::MaterialComponent>(entityId);
		materialData->color = materialComp.albedoColor;
		modelRenderComp.materialResourceHandle = static_cast<uint32_t>(materialBufferHandle);

		// 頂点バッファの更新
		if (vertexBufferMap.find(modelRenderComp.modelName) != vertexBufferMap.end()) {
			modelRenderComp.vertexResourceHandle = static_cast<uint32_t>(vertexBufferMap[modelRenderComp.modelName]);
		} else {
			if (loadModelVertexBufferFunc(modelRenderComp.modelName)) {
				modelRenderComp.vertexResourceHandle = static_cast<uint32_t>(vertexBufferMap[modelRenderComp.modelName]);
			} else {
				modelRenderComp.renderErrorMessage = "Failed to load vertex buffer for model: " + modelRenderComp.modelName;
				return;
			}
		}

		// テクスチャの更新
		QFE::GRAPHIC::DirectXResourceHandle textureHandle;
		QFE::FRAMEWORK::GetWhite1x1TextureHandle(graphicEngine, textureHandle);
		modelRenderComp.textureResourceHandle = static_cast<uint32_t>(textureHandle);

		// レイトレーシングインスタンスの作成
		if (blasHandleMap.find(modelRenderComp.modelName) == blasHandleMap.end()) {
			if (!loadBlasFunc(modelRenderComp.modelName)) {
				modelRenderComp.renderErrorMessage = "Failed to load BLAS for model: " + modelRenderComp.modelName;
				return;
			}
		}
		raytracingInstances.push_back({
			blasHandleMap[modelRenderComp.modelName],
			QFE::MATH::Matrix4x4::MakeAffineMatrix(entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform)
			});

		// レンダリング可能
		modelRenderComp.canRender = true;
		modelRenderComp.renderErrorMessage = "";
		});
	return false;
}
