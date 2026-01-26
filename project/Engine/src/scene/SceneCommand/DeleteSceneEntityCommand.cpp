#include "engine/include/scene/SceneCommand/DeleteSceneEntityCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"
#include "Engine/include/graphic/GpuBufferPool/GpuBufferPool.h"
#include "engine/include/assets/Script/CsharpVirtualEnvironmentOnQFE.h"

#include "Engine/include/assets/3DModel/Data/ModelHandle.h"
#include "Engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"
#include "Engine/include/assets/Particle/Data/ParticleComponent.h"
#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"

DeleteSceneEntityCommand::DeleteSceneEntityCommand(EntityManager& entityManager, uint32_t entityId)
:ISceneEntityCommand(entityManager),entityId_(entityId){}

void DeleteSceneEntityCommand::Execute()
{
	// エンティティのコンポーネント情報をシリアライズして保存する
	serializedEntityJson_ = entityManager_.SerializeEntityComponents(entityId_);

	// GPUリソースの解放など、必要なクリーンアップ処理を行う
	AssetManager* assetManager = AssetManager::GetInstance();
	GpuBufferPool* gpuBufferPool = assetManager->GetGpuBufferPool();

	// ModelコンポーネントのGPUリソース解放
	if (entityManager_.HasComponent<ModelHandle>(entityId_)) {
		ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId_);
		const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
		for (const auto& meshData : modelData->meshRenderDataHandles) {
			gpuBufferPool->ReleaseConstantBuffer<TransformationMatrix>(meshData.wpvBufferHandle);
			gpuBufferPool->ReleaseConstantBuffer<Material>(meshData.materialHandle);
			gpuBufferPool->ReleaseConstantBuffer<DirectionalLight>(meshData.lightBufferHandle);
		}
	}
	// SpriteコンポーネントのGPUリソース解放
	if (entityManager_.HasComponent<SpriteData>(entityId_)) {
		SpriteData& spriteData = entityManager_.GetComponent<SpriteData>(entityId_);
		gpuBufferPool->ReleaseConstantBuffer<TransformationMatrix>(spriteData.wvpBufferHandle);
		gpuBufferPool->ReleaseConstantBuffer<Material>(spriteData.materialBufferHandle);
	}

	// C#Scriptコンポーネントのインスタンス削除
	if (entityManager_.HasComponent<CsharpComponent>(entityId_)) {
		CsharpVirtualEnvironmentOnQFE* csharpEnv = CsharpVirtualEnvironmentOnQFE::GetInstance();
		for (const auto& handle : entityManager_.GetComponent<CsharpComponent>(entityId_).csharpHandles_) {
			csharpEnv->DeleteScriptInstance(handle.scriptIndex_);
		}
	}

	// 指定されたエンティティをシーンから削除する
	entityManager_.RemoveEntity(entityId_);
}

void DeleteSceneEntityCommand::Undo()
{
	/// 削除されたエンティティをシーンに復元する
}

std::string DeleteSceneEntityCommand::GetCommandName() const
{
	return "DeleteSceneEntityCommand";
}
