#include "engine/include/scene/SceneCommand/DeleteSceneEntityCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"
#include "Engine/include/graphic/GpuBufferPool/GpuBufferPool.h"


#include "Engine/include/assets/3DModel/Data/ModelHandle.h"
#include "Engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "engine/include/assets/3DModel/Data/SkyboxComponent.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"
#include "Engine/include/assets/Particle/Data/ParticleComponent.h"
#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"

using namespace QFE;

QFE::DeleteSceneEntityCommand::DeleteSceneEntityCommand(
	EntityManager& entityManager,
	CsharpScriptExecutor& scriptExecutor,
	uint32_t entityId)
	: ISceneEntityCommand(entityManager), entityId_(entityId), scriptExecutor_(scriptExecutor) {
}

void DeleteSceneEntityCommand::Execute()
{
	QFE_LOG("Executing DeleteSceneEntityCommand for entity " + std::to_string(entityId_));

	// エンティティのコンポーネント情報をシリアライズして保存する
	serializedEntityJson_ = entityManager_.SerializeEntityComponents(entityId_);

	// GPUリソースの解放など、必要なクリーンアップ処理を行う
	AssetManager* assetManager = AssetManager::GetInstance();
	GpuBufferPool* gpuBufferPool = 
		assetManager->GetGpuBufferPool();

	// ModelコンポーネントのGPUリソース解放
	if (entityManager_.HasComponent<ModelHandle>(entityId_)) {
		QFE_LOG("Releasing GPU resources for Model component of entity " + std::to_string(entityId_));
		ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId_);
		const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
		for (const auto& meshData : modelData->meshRenderDataHandles) {
			QFE_LOG("Releasing GPU resources for MeshRenderDataHandle of entity " + std::to_string(entityId_));
			gpuBufferPool->ReleaseConstantBuffer<TransformationMatrix>(meshData.wpvBufferHandle);
			gpuBufferPool->ReleaseConstantBuffer<Material>(meshData.materialHandle);
			gpuBufferPool->ReleaseConstantBuffer<DirectionalLight>(meshData.lightBufferHandle);
		}
	}
	// SkyboxコンポーネントのGPUリソース解放
	if (entityManager_.HasComponent<SkyboxComponent>(entityId_)) {
		QFE_LOG("Releasing GPU resources for Skybox component of entity " + std::to_string(entityId_));
		SkyboxComponent& skyboxComponent = entityManager_.GetComponent<SkyboxComponent>(entityId_);
		gpuBufferPool->ReleaseConstantBuffer<TransformationMatrix>(skyboxComponent.wvpBufferHandle);
		gpuBufferPool->ReleaseConstantBuffer<Material>(skyboxComponent.materialBufferHandle);
	}
	// SpriteコンポーネントのGPUリソース解放
	if (entityManager_.HasComponent<SpriteData>(entityId_)) {
		QFE_LOG("Releasing GPU resources for Sprite component of entity " + std::to_string(entityId_));
		SpriteData& spriteData = entityManager_.GetComponent<SpriteData>(entityId_);
		gpuBufferPool->ReleaseConstantBuffer<TransformationMatrix>(spriteData.wvpBufferHandle);
		gpuBufferPool->ReleaseConstantBuffer<Material>(spriteData.materialBufferHandle);
	}

	// 指定されたエンティティをシーンから削除する
	entityManager_.RemoveEntity(entityId_);
	QFE_LOG("Entity " + std::to_string(entityId_) + " marked for deletion");
}

void DeleteSceneEntityCommand::Undo()
{
	/// 削除されたエンティティをシーンに復元する
}

std::string DeleteSceneEntityCommand::GetCommandName() const
{
	return "DeleteSceneEntityCommand";
}
