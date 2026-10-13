#include "DeleteComponentCommand.h"
#include "design-patterns/EntityManager.h"

QFE::EDITOR::DeleteComponentCommand::DeleteComponentCommand(uint32_t entityId, EntityManager* entityManager, const std::string& componentName) :
	entityManager_(entityManager), entityId_(entityId), deleteComponentName_(componentName) {
}

void QFE::EDITOR::DeleteComponentCommand::Execute() {
	removedComponentData_ = entityManager_->SerializeComponent(entityId_, deleteComponentName_);
	entityManager_->RemoveComponent(entityId_, deleteComponentName_.c_str());
}

void QFE::EDITOR::DeleteComponentCommand::Undo() {
	entityManager_->DeserializeEntityComponents(entityId_, removedComponentData_);
}
