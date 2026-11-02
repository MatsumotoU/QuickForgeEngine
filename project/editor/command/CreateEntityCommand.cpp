#include "CreateEntityCommand.h"
#include "ecs/entity/EntityManager.h"
#include "components/MaterialComponent.h"
#include "components/ModelRenderComponent.h"
#include "components/SkeletonAnimationComponent.h"
#include "components/SpriteRenderComponent.h"
#include "components/ObjectInfoComponent.h"
#include "components/TransformComponent.h"
#include "components/ShaderComponent.h"
#include "components/SkyBoxComponent.h"

#include <utility>
#include <algorithm>
#include <cctype>
#include <filesystem>

QFE::EDITOR::CreateEntityCommand::CreateEntityCommand(std::string entityName,
	QFE::MATH::Vector3 position, EntityManager* entityManager, std::string modelName, bool createSprite, bool createSkyBox,
	std::string spriteTextureName, std::string modelAssetId, std::string spriteTextureAssetId) :
		entityManager_(entityManager), entityId_(UINT32_MAX),
		entityName_(std::move(entityName)), modelName_(std::move(modelName)), modelAssetId_(std::move(modelAssetId)),
		spriteTextureName_(std::move(spriteTextureName)), spriteTextureAssetId_(std::move(spriteTextureAssetId)),
		createSprite_(createSprite), createSkyBox_(createSkyBox), position_(position) {
}

void QFE::EDITOR::CreateEntityCommand::Execute() {
	// 新しいエンティティを作成する
	entityId_ = entityManager_->CreateEntity();
	// ObjectInfoComponentとTransformComponentを作成する
	QFE::SCENE::ObjectInfoComponent objectInfoComp;
	objectInfoComp.name = entityName_;
	QFE::SCENE::TransformComponent transformComp;
	transformComp.transform.translate = position_;
	// エンティティにコンポーネントを追加する
	entityManager_->EmplaceComponent<QFE::SCENE::ObjectInfoComponent>(entityId_, objectInfoComp);
	entityManager_->EmplaceComponent<QFE::SCENE::TransformComponent>(entityId_, transformComp);

	if (!modelName_.empty()) {
		QFE::SCENE::ModelRenderComponent modelRenderComp;
		modelRenderComp.modelName = modelName_;
		modelRenderComp.modelAssetId = modelAssetId_;
		entityManager_->EmplaceComponent<QFE::SCENE::ModelRenderComponent>(entityId_, modelRenderComp);
		std::string extension = std::filesystem::path(modelName_).extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (extension == ".glb") {
			// GLBから作成したEntityは、アニメーションがあれば従来どおり再生できる。
			entityManager_->EmplaceComponent<QFE::SCENE::SkeletonAnimationComponent>(entityId_);
		}
		entityManager_->EmplaceComponent<QFE::SCENE::ShaderComponent>(entityId_);

		QFE::SCENE::MaterialComponent materialComp{};
		materialComp.albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialComp.metallic = 0.0f;
		materialComp.smoothness = 0.5f;
		entityManager_->EmplaceComponent<QFE::SCENE::MaterialComponent>(entityId_, materialComp);
	}

	if (createSprite_) {
		transformComp.transform.scale = { 100.0f, 100.0f, 1.0f };
		entityManager_->GetComponent<QFE::SCENE::TransformComponent>(entityId_) = transformComp;
		QFE::SCENE::SpriteRenderComponent sprite;
		sprite.textureName = spriteTextureName_;
		sprite.textureAssetId = spriteTextureAssetId_;
		entityManager_->EmplaceComponent<QFE::SCENE::SpriteRenderComponent>(entityId_, sprite);
		entityManager_->EmplaceComponent<QFE::SCENE::MaterialComponent>(
			entityId_, QFE::SCENE::MaterialComponent{});
	}
	if (createSkyBox_) {
		entityManager_->EmplaceComponent<QFE::SCENE::SkyBoxComponent>(entityId_);
	}
}

void QFE::EDITOR::CreateEntityCommand::Undo() {
	if(entityId_ == UINT32_MAX) {
		return;
	}
	entityManager_->RemoveEntity(entityId_);
}
