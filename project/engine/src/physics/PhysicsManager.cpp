#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/Math/Transform.h"

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

		// 驥榊鴨
		if (forceComp.isGravity) {
			forceComp.acceleration.y += -9.8f * QFE::EngineGlobalValue::deltaTime * forceComp.gravityStrength; 
		}
		// 騾溷ｺｦ縺ｫ蜉帙ｒ蜉縺医ｋ
		forceComp.velocity += forceComp.acceleration * QFE::EngineGlobalValue::deltaTime;
		// 菴咲ｽｮ縺ｫ騾溷ｺｦ繧貞刈縺医ｋ
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& transform = entityManager->GetComponent<Transform>(entityId);
			transform.translate += forceComp.velocity * QFE::EngineGlobalValue::deltaTime;
		}
		// 鞫ｩ謫ｦ蜉帙・險育ｮ・
		forceComp.velocity = forceComp.velocity * (1.0f - forceComp.friction * QFE::EngineGlobalValue::deltaTime);
		forceComp.acceleration = forceComp.acceleration * (1.0f - forceComp.friction * QFE::EngineGlobalValue::deltaTime);
	}
}

void PhysicsManager::Finalize() {
}
