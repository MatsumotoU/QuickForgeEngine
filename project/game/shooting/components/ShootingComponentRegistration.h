#pragma once

#include "AllShootingComponents.h"
#include "core/design-patterns/component/StaticComponentRegistry.h"

namespace QFE::GAME {
	/// @brief Shootingゲーム固有のコンポーネントを登録する。
	inline bool RegisterShootingGameComponents(StaticComponentRegistry& registry) {
		return
			registry.RegisterComponent<STG::BulletEmitterComponent>("BulletEmitterComponent") &&
			registry.RegisterComponent<STG::InputBulletEmitterTriggerComponent>("InputBulletEmitterTriggerComponent") &&
			registry.RegisterComponent<STG::PeriodicBulletEmitterTriggerComponent>("PeriodicBulletEmitterTriggerComponent") &&
			registry.RegisterComponent<STG::ShootingPlayerComponent>("ShootingPlayerComponent") &&
			registry.RegisterComponent<STG::BulletComponent>("BulletComponent") &&
			registry.RegisterComponent<STG::HealthComponent>("HealthComponent") &&
			registry.RegisterComponent<STG::MoveLimitComponent>("MoveLimitComponent") &&
			registry.RegisterComponent<STG::PlayerTrackingComponent>("PlayerTrackingComponent") &&
			registry.RegisterComponent<STG::AutoScrollComponent>("AutoScrollComponent") &&
			registry.RegisterComponent<STG::EnemyAIComponent>("EnemyAIComponent") &&
			registry.RegisterComponent<STG::EntitySpawnerComponent>("EntitySpawnerComponent") &&
			registry.RegisterComponent<STG::SpawnTriggerComponent>("SpawnTriggerComponent");
	}
}
