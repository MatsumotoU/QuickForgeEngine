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

using namespace QFE;

AllEntityRenderingCommand::AllEntityRenderingCommand(EntityManager& entityManager) :
	ISceneEntityCommand(entityManager){}

void AllEntityRenderingCommand::Execute()
{
	// Particleの描画
	entityManager_.Each<ParticleComponent>([&](uint32_t entityId, ParticleComponent& particle) {
		particle;
		Render::Particle::DrawParticles(&entityManager_, entityId);
		});

	// モデルの描画
	entityManager_.Each<ModelHandle>([&](uint32_t entityId, ModelHandle& model) {
		entityId; // 未使用
		Render::Model::DrawModel(model.handle);
		});

	// スプライトの描画
	entityManager_.Each<SpriteData>([&](uint32_t entityId, SpriteData& sprite) {
		entityId; // 未使用
		Render::Sprite::DrawSprites(&entityManager_, entityId);
		});
}

void AllEntityRenderingCommand::Undo()
{
	// レンダリングコマンドは元に戻せない
}

std::string AllEntityRenderingCommand::GetCommandName() const
{
	return "AllEntityRenderingCommand";
}
