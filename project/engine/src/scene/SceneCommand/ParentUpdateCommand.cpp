#include "engine/include/scene/SceneCommand/ParentUpdateCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"

#include "engine/include/scene/Data/SceneObjectData.h"
#include "Engine/include/core/Math/ParentData.h"
#include "engine/include/core/Math/Transform.h"
#include "Engine/include/assets/3DModel/Data/ModelHandle.h"
#include "Engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"

using namespace QFE;

ParentUpdateCommand::ParentUpdateCommand(EntityManager& entityManager) : ISceneEntityCommand(entityManager) {}

void ParentUpdateCommand::Execute()
{
	AssetManager* assetManager = AssetManager::GetInstance();
	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<ParentData>(entityId)) {
			ParentData& parentData = entityManager_.GetComponent<ParentData>(entityId);

			uint32_t parentId = 0;
			bool isFound = false;
			if (entityManager_.HasComponentStrage<SceneObjectData>()) {
				auto& strage = entityManager_.GetComponentStrage<SceneObjectData>();
				for (const auto& [id, sceneObjData] : strage) {
					if (sceneObjData.uniqueId == parentData.parentId) {
						parentId = id;
						isFound = true;
						break;
					}
				}
			}
			if (!isFound) { continue; }

			if (entityManager_.HasComponent<Transform>(parentId)) {
				Transform& parentTransform = entityManager_.GetComponent<Transform>(parentId);
				// Modelのワールド行列更新
				if (entityManager_.HasComponent<ModelHandle>(entityId)) {
					ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId);
					const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
					for (const auto& meshData : modelData->meshRenderDataHandles) {
						TransformationMatrix* wpvMatrix = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(meshData.wpvBufferHandle);
						wpvMatrix->World = Matrix4x4::Multiply(wpvMatrix->World, Matrix4x4::MakeAffineMatrix(
							parentTransform.scale, parentTransform.rotate, parentTransform.translate));
					}
				}
				// スプライトのワールド行列更新
				if (entityManager_.HasComponent<SpriteData>(entityId)) {
					SpriteData& spriteData = entityManager_.GetComponent<SpriteData>(entityId);
					TransformationMatrix* wpvMatrix = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(spriteData.wvpBufferHandle);
					wpvMatrix->World = Matrix4x4::Multiply(wpvMatrix->World, Matrix4x4::MakeAffineMatrix(
						parentTransform.scale, parentTransform.rotate, parentTransform.translate));
				}
			}

		}
	}
}

void ParentUpdateCommand::Undo()
{
	// 親子関係の更新は元に戻せない
}

std::string ParentUpdateCommand::GetCommandName() const
{
	return "Parent Update Command";
}
