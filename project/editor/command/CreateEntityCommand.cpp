#include "CreateEntityCommand.h"
#include "design-patterns/EntityManager.h"
#include "components/MaterialComponent.h"
#include "components/ModelRenderComponent.h"
#include "components/ObjectInfoComponent.h"
#include "components/TransformComponent.h"

#include <utility>

QFE::EDITOR::CreateEntityCommand::CreateEntityCommand(std::string entityName,
	QFE::MATH::Vector3 position, EntityManager* entityManager, std::string modelName) :
		entityManager_(entityManager), entityId_(UINT32_MAX),
		entityName_(std::move(entityName)), modelName_(std::move(modelName)), position_(position) {
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
		entityManager_->EmplaceComponent<QFE::SCENE::ModelRenderComponent>(entityId_, modelRenderComp);

		QFE::SCENE::MaterialComponent materialComp{};
		materialComp.albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialComp.metallic = 0.0f;
		materialComp.smoothness = 0.5f;
		entityManager_->EmplaceComponent<QFE::SCENE::MaterialComponent>(entityId_, materialComp);
	}
}

void QFE::EDITOR::CreateEntityCommand::Undo() {
	if(entityId_ == UINT32_MAX) {
		return;
	}
	entityManager_->RemoveEntity(entityId_);
}
