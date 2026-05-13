#include "engine/include/scene/SceneCommand/AllSpriteResizeCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"

using namespace QFE;

AllSpriteResizeCommand::AllSpriteResizeCommand(EntityManager& em) : ISceneEntityCommand(em) {}

void AllSpriteResizeCommand::Execute() {
	AssetManager* assetManager_ = AssetManager::GetInstance();

	entityManager_.Each<SpriteData>([&](uint32_t entityId, SpriteData& spriteData) {
		entityId; // 未使用
		Vector2 nowSize = assetManager_->GetSpriteManager()->GetSpriteSize(spriteData.vertexBufferHandle);
		if (spriteData.width != nowSize.x || spriteData.height != nowSize.y) {
			assetManager_->GetSpriteManager()->ResizeSprite(spriteData.vertexBufferHandle, spriteData.width, spriteData.height);
		}
		});
}

void AllSpriteResizeCommand::Undo() {
	/// スプライトのリサイズは元に戻せない
}
