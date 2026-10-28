#pragma once

#include "AllShootingComponents.h"
#include "core/design-patterns/component/StaticComponentRegistry.h"

namespace QFE::GAME {
	/// @brief Shootingゲーム固有のコンポーネントを登録する。
	inline bool RegisterShootingGameComponents(StaticComponentRegistry& registry) {
		return
			registry.RegisterComponent<STG::BulletEmitterComponent>("BulletEmitterComponent", "Gameplay") &&
			registry.RegisterComponent<STG::InputBulletEmitterTriggerComponent>("InputBulletEmitterTriggerComponent", "Gameplay") &&
			registry.RegisterComponent<STG::PeriodicBulletEmitterTriggerComponent>("PeriodicBulletEmitterTriggerComponent", "Gameplay") &&
			registry.RegisterComponent<STG::ShootingPlayerComponent>("ShootingPlayerComponent", "Gameplay") &&
			registry.RegisterComponent<STG::BulletComponent>("BulletComponent", "Gameplay") &&
			registry.RegisterComponent<STG::HealthComponent>("HealthComponent", "Gameplay") &&
			registry.RegisterComponent<STG::MoveLimitComponent>("MoveLimitComponent", "Gameplay") &&
			registry.RegisterComponent<STG::PlayerTrackingComponent>("PlayerTrackingComponent", "Gameplay") &&
			registry.RegisterComponent<STG::AutoScrollComponent>("AutoScrollComponent", "Gameplay") &&
			registry.RegisterComponent<STG::EnemyAIComponent>("EnemyAIComponent", "Gameplay") &&
			registry.RegisterComponent<STG::EntitySpawnerComponent>("EntitySpawnerComponent", "Gameplay") &&
			registry.RegisterComponent<STG::SpawnTriggerComponent>("SpawnTriggerComponent", "Gameplay");
	}
}
