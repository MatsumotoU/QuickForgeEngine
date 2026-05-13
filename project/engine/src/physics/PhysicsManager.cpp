/**
 * @file PhysicsManager.cpp
 * @brief 物理演算の更新処理を統括するクラスの実装
 */

#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/Math/Transform.h"

using namespace QFE;

/** @brief 初期化 */
void PhysicsManager::Initialize() {
}

/** @brief 更新 */
void PhysicsManager::Update() {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();

	entityManager->Each<Force>([&](uint32_t entityId, Force& forceComp) {
		// 加速度のリセット
		forceComp.acceleration = Vector3::Zero();
		// 重力
		if (forceComp.isGravity) {
			forceComp.acceleration.y += -9.8f * EngineGlobalValue::deltaTime * forceComp.gravityStrength; 
		}
		// 速度に力を加える
		forceComp.velocity += forceComp.acceleration * EngineGlobalValue::deltaTime;
		// 位置に速度を加える
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& transform = entityManager->GetComponent<Transform>(entityId);
			transform.translate += forceComp.velocity * EngineGlobalValue::deltaTime;
		}
		// 摩擦力の計算
		float frictionFactor = std::exp(-forceComp.friction * EngineGlobalValue::deltaTime);
		forceComp.velocity = forceComp.velocity * frictionFactor;
		});
}

/** @brief 終了処理 */
void PhysicsManager::Finalize() {
}
