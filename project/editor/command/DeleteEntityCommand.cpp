#include "DeleteEntityCommand.h"
#include "design-patterns/EntityManager.h"

QFE::EDITOR::DeleteEntityCommand::DeleteEntityCommand(uint32_t entityId, QFE::EntityManager* entityManager) :
	entityId_(entityId), entityManager_(entityManager) {
	removedComponents_.clear();
}

void QFE::EDITOR::DeleteEntityCommand::Execute() {
	// エンティティが持っているコンポーネントの情報を取得して保存する
	removedComponents_ = entityManager_->SerializeEntityComponents(entityId_);
	// エンティティを削除する
	entityManager_->RemoveEntity(entityId_);
}

void QFE::EDITOR::DeleteEntityCommand::Undo() {
	// エンティティを再作成する
	entityManager_->ForceCreateEntity(entityId_);
	// 保存しておいたコンポーネントの情報を復元する
	entityManager_->DeserializeEntityComponents(entityId_, removedComponents_);
}
