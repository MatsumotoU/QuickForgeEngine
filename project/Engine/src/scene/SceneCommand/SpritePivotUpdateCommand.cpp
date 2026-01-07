#include "Engine/include/scene/SceneCommand/SpritePivotUpdateCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"

#include "Engine/include/assets/Sprite/Data/SpriteData.h"
SpritePivotUpdateCommand::SpritePivotUpdateCommand(EntityManager& entityManager) : ISceneEntityCommand(entityManager){}

void SpritePivotUpdateCommand::Execute()
{
	AssetManager* assetManager = AssetManager::GetInstance();
	if (entityManager_.HasComponentStrage<SpriteData>()) {
		auto& strage = entityManager_.GetComponentStrage<SpriteData>();
		for (auto& [id, data] : strage) {
			VertexData* vertexData = assetManager->GetSpriteManager()->GetVertexData(data.vertexBufferHandle);
			float w = data.width;
			float h = data.height;
			// 中心を基準に頂点座標設定
			Vector2 pivotOffset = Vector2(0.0f, 0.0f);
			pivotOffset.x = -w * data.pivot.x;
			pivotOffset.y = -h * data.pivot.y;
			vertexData[0].position = { pivotOffset.x, pivotOffset.y, 0.0f,1.0f };
			vertexData[1].position = { w + pivotOffset.x, pivotOffset.y, 0.0f ,1.0f };
			vertexData[2].position = { pivotOffset.x, h + pivotOffset.y, 0.0f ,1.0f };
			vertexData[3].position = { w + pivotOffset.x, h + pivotOffset.y, 0.0f ,1.0f };
			vertexData[4].position = { pivotOffset.x, h + pivotOffset.y, 0.0f,1.0f };
			vertexData[5].position = { w + pivotOffset.x, pivotOffset.y, 0.0f ,1.0f };
		}
	}
}

void SpritePivotUpdateCommand::Undo()
{
}

std::string SpritePivotUpdateCommand::GetCommandName() const
{
	return "Sprite Pivot Update Command";
}
