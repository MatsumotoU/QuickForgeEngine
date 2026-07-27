#include "CopyEntityCommand.h"
#include "design-patterns/EntityManager.h"
#include "components/ObjectInfoComponent.h"
#include "design-patterns/component/EntityUuid.h"

QFE::EDITOR::CopyEntityCommand::CopyEntityCommand(uint32_t entityId, EntityManager* entityManager) :
	entityId_(entityId),newEntityId_(UINT32_MAX), entityManager_(entityManager) {
}

void QFE::EDITOR::CopyEntityCommand::Execute() {
	newEntityId_ = entityManager_->CreateEntity();
	entityManager_->DeserializeEntityComponents(newEntityId_, entityManager_->SerializeEntityComponents(entityId_));
	if (entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(newEntityId_)) {
		entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(newEntityId_).uuid =
			QFE::GenerateEntityUuid();
	}
}

void QFE::EDITOR::CopyEntityCommand::Undo() {
	if (newEntityId_ != UINT32_MAX) {
		entityManager_->RemoveEntity(newEntityId_);
		newEntityId_ = UINT32_MAX; // コピーされたエンティティIDをリセット
	}
}
