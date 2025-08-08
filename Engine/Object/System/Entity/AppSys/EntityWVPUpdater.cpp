#include "EntityWVPUpdater.h"
#include "Object/Entity/EntityManager.h"
#include "Camera/Camera.h"

void EntityWVPUpdater::Update(EntityManager& entityManager, Camera& camera) {
	// EntityManagerからTransformComponentを取得し、WVP行列を更新
	if (!entityManager.HasComponentStrage<TransformComponent>()) {
		return;
	}

	auto& transformStrage = entityManager.GetComponentStrage<TransformComponent>();
	for (auto& [entityId, transform] : transformStrage) {
		TransformComponent& transformComponent = entityManager.GetComponent<TransformComponent>(entityId);
		MakeWVPForTransformComponent::Make(transformComponent, camera);
	}
}
