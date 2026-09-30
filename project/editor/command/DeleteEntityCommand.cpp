#include "DeleteEntityCommand.h"
#include "design-patterns/EntityManager.h"

QFE::EDITOR::DeleteEntityCommand::DeleteEntityCommand(uint32_t entityId, QFE::EntityManager* entityManager) :
	entityId_(entityId), entityManager_(entityManager) {
	removedEntities_.clear();
}

void QFE::EDITOR::DeleteEntityCommand::Execute() {
	removedEntities_.clear();
	std::vector<uint32_t> entityIds{ entityId_ };
	const std::vector<uint32_t> descendants = entityManager_->GetDescendantEntityIds(entityId_);
	entityIds.insert(entityIds.end(), descendants.begin(), descendants.end());

	for (const uint32_t entityId : entityIds) {
		removedEntities_.emplace_back(entityId, entityManager_->SerializeEntityComponents(entityId));
	}

	// エンティティを削除する
	entityManager_->RemoveEntity(entityId_);
}

void QFE::EDITOR::DeleteEntityCommand::Undo() {
	for (const auto& removedEntity : removedEntities_) {
		entityManager_->CancelEntityRemoval(removedEntity.first);
		entityManager_->ForceCreateEntity(removedEntity.first);
	}
	for (const auto& removedEntity : removedEntities_) {
		entityManager_->DeserializeEntityComponents(removedEntity.first, removedEntity.second);
	}
}
