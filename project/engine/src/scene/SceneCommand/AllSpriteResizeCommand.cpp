#include "engine/include/scene/SceneCommand/AllSpriteResizeCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"

using namespace QFE;

AllSpriteResizeCommand::AllSpriteResizeCommand(EntityManager& em) : ISceneEntityCommand(em) {}

void AllSpriteResizeCommand::Execute() {
	AssetManager* assetManager_ = AssetManager::GetInstance();
	if (entityManager_.HasComponentStrage<SpriteData>()) {
		const auto& spriteStrage = entityManager_.GetComponentStrage<SpriteData>();
		for (const auto& [entityId, sprite] : spriteStrage) {
			Vector2 nowSize = assetManager_->GetSpriteManager()->GetSpriteSize(sprite.vertexBufferHandle);
			if (sprite.width != nowSize.x || sprite.height != nowSize.y) {
				assetManager_->GetSpriteManager()->ResizeSprite(sprite.vertexBufferHandle, sprite.width, sprite.height);
			}
		}
	}
}

void AllSpriteResizeCommand::Undo() {
	/// スプライトのリサイズは元に戻せない
}
