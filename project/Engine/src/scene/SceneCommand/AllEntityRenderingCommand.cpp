#include "engine/include/scene/SceneCommand/AllEntityRenderingCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"

#include "Engine/include/assets/3DModel/Data/ModelHandle.h"
#include "Engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"
#include "Engine/include/assets/Particle/Data/ParticleComponent.h"

#include "Engine/include/renderer/ModelRenderer.h"
#include "Engine/include/renderer/SpriteRenderer.h"
#include "Engine/include/renderer/ParticleRenderer.h"
AllEntityRenderingCommand::AllEntityRenderingCommand(EntityManager& entityManager) :
	ISceneEntityCommand(entityManager){}

void AllEntityRenderingCommand::Execute()
{
	// Particleの描画
	if (entityManager_.HasComponentStrage<ParticleComponent>()) {
		const auto& particleStrage = entityManager_.GetComponentStrage<ParticleComponent>();
		for (const auto& [entityId, particle] : particleStrage) {
			Render::Particle::DrawParticles(entityId);
		}
	}
	// モデルの描画
	if (entityManager_.HasComponentStrage<ModelHandle>()) {
		const auto& modelStrage = entityManager_.GetComponentStrage<ModelHandle>();
		for (const auto& [entityId, model] : modelStrage) {
			Render::Model::DrawModel(model.handle);
		}
	}
	// スプライトの描画（レイヤー順）
	if (entityManager_.HasComponentStrage<SpriteData>()) {
		const auto& spriteStrage = entityManager_.GetComponentStrage<SpriteData>();
		std::vector<std::pair<uint32_t, SpriteData>> sortedSprites(spriteStrage.begin(), spriteStrage.end());
		std::sort(sortedSprites.begin(), sortedSprites.end(),
			[](const auto& a, const auto& b) {
				return a.second.layer < b.second.layer;
			});
		for (const auto& [entityId, sprite] : sortedSprites) {
			Render::Sprite::DrawSprite(entityId);
		}
	}
}

void AllEntityRenderingCommand::Undo()
{
	// レンダリングコマンドは元に戻せない
}

std::string AllEntityRenderingCommand::GetCommandName() const
{
	return "AllEntityRenderingCommand";
}
