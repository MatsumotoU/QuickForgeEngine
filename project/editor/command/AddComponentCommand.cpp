#include "AddComponentCommand.h"
#include "design-patterns/EntityManager.h"

QFE::EDITOR::AddComponentCommand::AddComponentCommand(uint32_t entityId, EntityManager* entityManager, const std::string& componentName) :
	entityManager_(entityManager), entityId_(entityId), addComponentName_(componentName) {
}

void QFE::EDITOR::AddComponentCommand::Execute() {
	entityManager_->AddDefaultComponent(entityId_, addComponentName_);
}

void QFE::EDITOR::AddComponentCommand::Undo() {
	entityManager_->DeleteComponent(entityId_, addComponentName_);
}
