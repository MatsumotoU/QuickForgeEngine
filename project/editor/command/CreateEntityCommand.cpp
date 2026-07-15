#include "CreateEntityCommand.h"
#include "design-patterns/EntityManager.h"
#include "components/ObjectInfoComponent.h"
#include "components/TransformComponent.h"

QFE::EDITOR::CreateEntityCommand::CreateEntityCommand(std::string entityName,
	QFE::MATH::Vector3 position, EntityManager* entityManager) :
		entityId_(UINT32_MAX), entityName_(entityName), position_(position), entityManager_(entityManager) {
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
}

void QFE::EDITOR::CreateEntityCommand::Undo() {
	if(entityId_ == UINT32_MAX) {
		return;
	}
	entityManager_->RemoveEntity(entityId_);
}
