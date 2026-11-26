#include "framework/application/GameModuleApi.h"

#include "game/shooting/components/ShootingComponentRegistration.h"
#include "game/shooting/systems/ShootingSystem.h"

#define QFE_GAME_EXPORT extern "C" __declspec(dllexport)

QFE_GAME_EXPORT uint32_t QFE_GetGameModuleApiVersion() {
	return QFE::FRAMEWORK::GameModuleApiVersion;
}

QFE_GAME_EXPORT bool QFE_RegisterGameComponents(QFE::StaticComponentRegistry& registry) {
	return QFE::GAME::RegisterShootingGameComponents(registry);
}

QFE_GAME_EXPORT void QFE_UpdateGame(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources,
	float deltaTime) {
	// ゲーム固有の処理だけを更新し、シーン共通処理はフレームワーク側に任せる。
	QFE::GAMESYSTEM::AutoScrollSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::InputMovementSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::PhysicsComponentSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::ShootingPlayerSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::ShootingEnemySystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::BulletEmitterTriggerSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::BulletEmitterSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::BulletUpdateSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::CollisionSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::MoveLimitSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::PlayerTrackingSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::EnemySpawnerSystem(systems, resources, deltaTime);
}
