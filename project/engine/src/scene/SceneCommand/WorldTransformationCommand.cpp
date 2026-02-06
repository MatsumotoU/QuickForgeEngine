#include "engine/include/scene/SceneCommand/WorldTransformationCommand.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"

#include "engine/include/core/Math/Transform.h"
#include "Engine/include/assets/3DModel/Data/ModelHandle.h"
#include "Engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"
#include "Engine/include/assets/Particle/Data/ParticleComponent.h"
using namespace QFE;
WorldTransformationCommand::WorldTransformationCommand(EntityManager& em) : ISceneEntityCommand(em) {}

void WorldTransformationCommand::Execute(){
	AssetManager* assetManager = AssetManager::GetInstance();
	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();
	//　ワールド行列更新
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<Transform>(entityId)) {
			Transform& transform = entityManager_.GetComponent<Transform>(entityId);
			// モデルのワールド行列更新
			if (entityManager_.HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId);
				const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
				// メッシュごとのワールド行列更新
				for (const auto& meshData : modelData->meshRenderDataHandles) {
					TransformationMatrix* wpvMatrix = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(meshData.wpvBufferHandle);
					wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
						transform.scale,
						transform.rotate,
						transform.translate
					);
				}
			}
			// スプライトのワールド行列更新
			if (entityManager_.HasComponent<SpriteData>(entityId)) {
				SpriteData& spriteData = entityManager_.GetComponent<SpriteData>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(spriteData.wvpBufferHandle);
				wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
					transform.scale,
					transform.rotate,
					transform.translate
				);
			}
			// パーティクルのワールド行列更新
			if (entityManager_.HasComponent<ParticleComponent>(entityId)) {
				ParticleComponent& particleComp = entityManager_.GetComponent<ParticleComponent>(entityId);
				ParticleForGPU* particleData = assetManager->GetParticleGpuDataManager()->GetDataPtr(particleComp.particleGpuBufferHandle);
				for (uint32_t i = 0; i < particleComp.maxParticleCount; i++) {
					particleData[i].World = Matrix4x4::MakeAffineMatrix(
						transform.scale,
						transform.rotate,
						transform.translate
					);
				}
			}
		}
	}
}

void WorldTransformationCommand::Undo() {
	// ワールド行列の更新は元に戻せない
}

std::string WorldTransformationCommand::GetCommandName() const
{
	return "WorldTransformationCommand";
}
