#include "BindEntityParent.h"
#include "Object/Entity/EntityManager.h"

void BindEntityParent::Bind(EntityManager& entityManager, uint32_t entityId, uint32_t parentEntityId) {
	if (!entityManager.HasComponent<ParentComponent>(entityId)) {
		entityManager.EmplaceComponent<ParentComponent>(entityId, ParentComponent{ parentEntityId });
	} else {
		auto& parentComponent = entityManager.GetComponent<ParentComponent>(entityId);
		parentComponent.parentEntityId = parentEntityId;
	}
}
