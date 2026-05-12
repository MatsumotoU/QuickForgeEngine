#include "Engine/include/scene/SceneCommand/WvpTransformationCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"

#include "Engine/include/assets/3DModel/Data/ModelHandle.h"
#include "Engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "engine/include/assets/3DModel/Data/SkyboxComponent.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"
#include "Engine/include/assets/Particle/Data/ParticleComponent.h"

using namespace QFE;

WvpTransformationCommand::WvpTransformationCommand(EntityManager& entityManager, CameraManager& cameraMana) :
	ISceneEntityCommand(entityManager),
	cameraManager_(cameraMana) {}

void WvpTransformationCommand::Execute()
{
	AssetManager* assetManager = AssetManager::GetInstance();
	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<Transform>(entityId)) {
			// ModelのWVP行列更新
			if (entityManager_.HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId);
				const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
				//MeshごとのWVP行列更新
				for (const auto& meshData : modelData->meshRenderDataHandles) {
					TransformationMatrix* wpvMatrix = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(meshData.wpvBufferHandle);
					wpvMatrix->WVP = cameraManager_.GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Perspective);
				}
			}
			// SkyboxのWVP行列更新
			if (entityManager_.HasComponent<SkyboxComponent>(entityId)) {
				SkyboxComponent& skyboxComp = entityManager_.GetComponent<SkyboxComponent>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(skyboxComp.wvpBufferHandle);
				wpvMatrix->WVP = cameraManager_.GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Perspective);
			}
			// SpriteのWVP行列更新
			if (entityManager_.HasComponent<SpriteData>(entityId)) {
				SpriteData& spriteData = entityManager_.GetComponent<SpriteData>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(spriteData.wvpBufferHandle);
				wpvMatrix->WVP = cameraManager_.GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Orthographic);
			}
			// ParticleのWVP行列更新
			if (entityManager_.HasComponent<ParticleComponent>(entityId)) {
				ParticleComponent& particleComp = entityManager_.GetComponent<ParticleComponent>(entityId);
				ParticleForGPU* particleData = assetManager->GetParticleGpuDataManager()->GetDataPtr(particleComp.particleGpuBufferHandle);
				for (uint32_t i = 0; i < particleComp.maxParticleCount; i++) {
					particleData[i].WVP = cameraManager_.GetMainCamera().GetWorldViewProjectionMatrix(particleData[i].World, CameraType::Perspective);
				}
			}
		}
	}
}

void WvpTransformationCommand::Undo()
{
}

std::string WvpTransformationCommand::GetCommandName() const
{
	return "Wvp Transformation Command";
}
