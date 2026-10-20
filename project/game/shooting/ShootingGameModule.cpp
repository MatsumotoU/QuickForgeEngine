#include "framework/application/GameModuleApi.h"

#include "game/shooting/components/ShootingComponentRegistration.h"
#include "game/shooting/systems/ShootingSystem.h"

#include "framework/animation/AnimationSystem.h"
#include "framework/scene/EventSystem.h"
#include "framework/scene/SceneChangeSystem.h"
#include "framework/scene/SplineMovementSystem.h"
#include "scene/SceneManager.h"

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
	QFE::SCENE::SceneManager& sceneManager = *systems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	QFE::GAMESYSTEM::AutoScrollSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::InputMovementSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::PhysicsComponentSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::ShootingPlayerSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::ShootingEnemySystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::BulletEmitterTriggerSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::BulletEmitterSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::BulletUpdateSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::CollisionSystem(systems, resources, deltaTime);
	QFE::FRAMEWORK::UpdateEventComponents(entityManager, deltaTime);
	QFE::FRAMEWORK::UpdateAnimationComponents(entityManager, deltaTime);
	QFE::GAMESYSTEM::MoveLimitSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::PlayerTrackingSystem(systems, resources, deltaTime);
	QFE::GAMESYSTEM::EnemySpawnerSystem(systems, resources, deltaTime);
	QFE::FRAMEWORK::UpdateSplineMovement(entityManager, deltaTime);
	systems.sceneChangeTransition.Update(sceneManager, deltaTime);
}
