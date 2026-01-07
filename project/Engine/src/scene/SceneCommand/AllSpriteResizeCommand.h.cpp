#include "engine/include/scene/SceneCommand/AllSpriteResizeCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"

AllSpriteResizeCommand::AllSpriteResizeCommand(EntityManager& em) : ISceneEntityCommand(em) {}

void AllSpriteResizeCommand::Execute()
{
	AssetManager* assetManager_ = AssetManager::GetInstance();
	if (assetManager_->GetEntityManager()->HasComponentStrage<SpriteData>()) {
		const auto& spriteStrage = assetManager_->GetEntityManager()->GetComponentStrage<SpriteData>();
		for (const auto& [entityId, sprite] : spriteStrage) {
			Vector2 nowSize = assetManager_->GetSpriteManager()->GetSpriteSize(sprite.vertexBufferHandle);
			if (sprite.width != nowSize.x || sprite.height != nowSize.y) {
				assetManager_->GetSpriteManager()->ResizeSprite(sprite.vertexBufferHandle, sprite.width, sprite.height);
			}
		}
	}
}

void AllSpriteResizeCommand::Undo()
{
	/// スプライトのリサイズは元に戻せない
}
