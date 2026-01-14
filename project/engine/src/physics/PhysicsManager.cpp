/**
 * @file PhysicsManager.cpp
 * @brief 物理演算の更新処理を統括するクラスの実装
 */

#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/Math/Transform.h"

/** @brief 初期化 */
void PhysicsManager::Initialize() {
}

/** @brief 更新 */
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
            // TODO: 加速度に deltaTime を掛けて加算しているが、通常の物理式では加速度そのものを変化させるべき。
            // また、Updateの最初で加速度をリセットしない設計の場合、力が蓄積し続ける可能性がある。
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
        // TODO: (1.0f - friction * deltaTime) による線形近似は、deltaTime が大きい場合に負の値になる可能性がある。
        // また、加速度に対しても摩擦を適用しているのは物理的に特殊な挙動であるため再検討が必要。
		forceComp.velocity = forceComp.velocity * (1.0f - forceComp.friction * QFE::EngineGlobalValue::deltaTime);
		forceComp.acceleration = forceComp.acceleration * (1.0f - forceComp.friction * QFE::EngineGlobalValue::deltaTime);
	}
}

/** @brief 終了処理 */
void PhysicsManager::Finalize() {
}
