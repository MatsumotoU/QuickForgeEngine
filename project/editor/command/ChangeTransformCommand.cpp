#include "ChangeTransformCommand.h"
#include "design-patterns/EntityManager.h"
#include "components/TransformComponent.h"

QFE::EDITOR::ChangeTransformCommand::ChangeTransformCommand(
	uint32_t entityId,
	EntityManager* entityManager,
	const QFE::MATH::EulerTransform& before,
	const QFE::MATH::EulerTransform& after) :
	entityId_(entityId),
	entityManager_(entityManager),
	before_(before),
	after_(after) {
}

void QFE::EDITOR::ChangeTransformCommand::Execute() {
	Apply(after_);
}

void QFE::EDITOR::ChangeTransformCommand::Undo() {
	Apply(before_);
}

void QFE::EDITOR::ChangeTransformCommand::Apply(const QFE::MATH::EulerTransform& transform) {
	if (entityManager_ == nullptr ||
		!entityManager_->HasComponent<QFE::SCENE::TransformComponent>(entityId_)) {
		return;
	}
	entityManager_->GetComponent<QFE::SCENE::TransformComponent>(entityId_).transform = transform;
}
