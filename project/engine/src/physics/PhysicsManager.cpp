#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/TimeManager.h"

void PhysicsManager::Initialize() {
}

void PhysicsManager::Update() {
	const float deltaTime = TimeManager::GetInstance()->GetDeltaTime();
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponentStrage<Force>()) {
		return;
	}

	const auto& forceStrage = entityManager->GetComponentStrage<Force>();
	for (const auto& force : forceStrage) {
		uint32_t entityId = force.first;
		Force& forceComp = entityManager->GetComponent<Force>(entityId);

		// 重力
		if (forceComp.isGravity) {
			forceComp.acceleration.y += -9.8f * deltaTime * forceComp.gravityStrength;
		}
		// 速度に力を加える
		forceComp.velocity += forceComp.acceleration * deltaTime;
		// 位置に速度を加える
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& transform = entityManager->GetComponent<Transform>(entityId);
			transform.translate += forceComp.velocity * deltaTime;
		}
		// 摩擦力の計算
		forceComp.velocity = forceComp.velocity * (1.0f - forceComp.friction * deltaTime);
		forceComp.acceleration = forceComp.acceleration * (1.0f - forceComp.friction * deltaTime);
	}
}

void PhysicsManager::Finalize() {
}
