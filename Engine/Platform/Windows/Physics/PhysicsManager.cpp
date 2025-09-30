#include "PhysicsManager.h"
#include "Assets/AssetManager.h"
#include "Core/EngineGlobalValue.h"
#include "Core/Math/Transform.h"

void PhysicsManager::Initialize() {
}

void PhysicsManager::Update() {
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
			forceComp.acceleration.y += -9.8f * QFE::EngineGlobalValue::deltaTime * forceComp.gravityStrength; 
		}
		// 速度に力を加える
		forceComp.velocity += forceComp.acceleration * QFE::EngineGlobalValue::deltaTime;
		// 位置に速度を加える
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& transform = entityManager->GetComponent<Transform>(entityId);
			transform.translate += forceComp.velocity * QFE::EngineGlobalValue::deltaTime;
		}
		// 摩擦力の計算
		forceComp.velocity = forceComp.velocity * (1.0f - forceComp.friction * QFE::EngineGlobalValue::deltaTime);
		forceComp.acceleration = forceComp.acceleration * (1.0f - forceComp.friction * QFE::EngineGlobalValue::deltaTime);
	}
}

void PhysicsManager::Finalize() {
}
