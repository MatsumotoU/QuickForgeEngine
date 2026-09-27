#include "ShootingSystem.h"

#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "components/TransformHierarchy.h"
#include "framework/scene/CollisionTriggerSystem.h"

#include <algorithm>
#include <vector>

namespace {
	QFE::MATH::Vector3 GetWorldPositionFromMatrix(const QFE::MATH::Matrix4x4& worldMatrix) {
		return {
			worldMatrix.m[3][0],
			worldMatrix.m[3][1],
			worldMatrix.m[3][2]
		};
	}

	QFE::MATH::Vector3 GetWorldPosition(
		const QFE::EntityManager& entityManager,
		uint32_t entityId) {
		return GetWorldPositionFromMatrix(QFE::SCENE::GetWorldMatrix(entityManager, entityId));
	}

	QFE::MATH::Vector3 GetWorldPositionWithOffset(
		const QFE::MATH::Matrix4x4& worldMatrix,
		const QFE::MATH::Vector3& localOffset) {
		QFE::MATH::Vector3 worldAxisX = {
			worldMatrix.m[0][0], worldMatrix.m[0][1], worldMatrix.m[0][2]
		};
		QFE::MATH::Vector3 worldAxisY = {
			worldMatrix.m[1][0], worldMatrix.m[1][1], worldMatrix.m[1][2]
		};
		QFE::MATH::Vector3 worldAxisZ = {
			worldMatrix.m[2][0], worldMatrix.m[2][1], worldMatrix.m[2][2]
		};
		// Follow the parent's rotation without letting scale change the configured spawn offset.
		worldAxisX = worldAxisX.Normalize();
		worldAxisY = worldAxisY.Normalize();
		worldAxisZ = worldAxisZ.Normalize();

		return GetWorldPositionFromMatrix(worldMatrix) +
			worldAxisX * localOffset.x +
			worldAxisY * localOffset.y +
			worldAxisZ * localOffset.z;
	}

	void RequestBulletEmitterPattern(
		QFE::EntityManager& entityManager,
		uint32_t entityId,
		uint32_t patternIndex) {
		if (entityManager.HasComponent<QFE::STG::BulletEmitterComponent>(entityId)) {
			entityManager.GetComponent<QFE::STG::BulletEmitterComponent>(entityId)
				.RequestEmit(patternIndex);
		}
	}
}

bool QFE::GAMESYSTEM::InputMovementSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	(void)resources;

	QFE::EntityManager& entityManager = systems.sceneManager->GetCurrentSceneEntityManager();
	QFE::INPUT::InputInterface* inputInterface = systems.inputInterface.get();
	if (inputInterface == nullptr || deltaTime <= 0.0f) {
		return false;
	}

	entityManager.Each<QFE::COMPONENTS::InputMovementComponent>(
		[&](uint32_t entityId, QFE::COMPONENTS::InputMovementComponent& movement) {
			if (!movement.enabled || movement.amount == 0.0f ||
				(entityManager.HasComponent<QFE::STG::ShootingPlayerComponent>(entityId) &&
					!entityManager.GetComponent<QFE::STG::ShootingPlayerComponent>(entityId).inputEnabled)) {
				return;
			}

			float horizontalInput = 0.0f;
			float verticalInput = 0.0f;
			if (!movement.rightActionName.empty() && inputInterface->GetKeyPress(movement.rightActionName)) {
				horizontalInput += 1.0f;
			}
			if (!movement.leftActionName.empty() && inputInterface->GetKeyPress(movement.leftActionName)) {
				horizontalInput -= 1.0f;
			}
			if (!movement.forwardActionName.empty() && inputInterface->GetKeyPress(movement.forwardActionName)) {
				verticalInput += 1.0f;
			}
			if (!movement.backwardActionName.empty() && inputInterface->GetKeyPress(movement.backwardActionName)) {
				verticalInput -= 1.0f;
			}
			if (movement.useGamePadLeftStick) {
				const QFE::MATH::Vector2 stickInput = inputInterface->GetGamePadLeftStickDir();
				horizontalInput += stickInput.x;
				verticalInput += stickInput.y;
			}

			horizontalInput = std::clamp(horizontalInput, -1.0f, 1.0f);
			verticalInput = std::clamp(verticalInput, -1.0f, 1.0f);
			QFE::MATH::Vector3 direction =
				movement.horizontalAxis * horizontalInput + movement.verticalAxis * verticalInput;
			if (direction.LengthSq() == 0.0f) {
				return;
			}
			if (movement.normalizeInput && direction.LengthSq() > 1.0f) {
				direction = direction.Normalize();
			}

			if (movement.applyMode == QFE::COMPONENTS::InputMovementForce) {
				if (!entityManager.HasComponent<QFE::COMPONENTS::PhysicsComponent>(entityId)) {
					return;
				}
				QFE::COMPONENTS::PhysicsComponent& physics =
					entityManager.GetComponent<QFE::COMPONENTS::PhysicsComponent>(entityId);
				if (physics.mass > 0.0f) {
					physics.acceleration += direction * (movement.amount / physics.mass);
				}
				return;
			}

			if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform.translate +=
					direction * movement.amount * deltaTime;
			}
		});

	return true;
}

bool QFE::GAMESYSTEM::PhysicsComponentSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	(void)resources;

	QFE::EntityManager& entityManager = systems.sceneManager->GetCurrentSceneEntityManager();
	if (deltaTime <= 0.0f) {
		return false;
	}

	entityManager.Each<QFE::COMPONENTS::PhysicsComponent>(
		[&](uint32_t entityId, QFE::COMPONENTS::PhysicsComponent& physics) {
			if (physics.mass <= 0.0f ||
				!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				physics.acceleration = QFE::MATH::Vector3::Zero();
				return;
			}

			physics.velocity += physics.acceleration * deltaTime;
			const float damping = std::clamp(1.0f - physics.friction * deltaTime, 0.0f, 1.0f);
			physics.velocity = physics.velocity * damping;
			entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform.translate +=
				physics.velocity * deltaTime;
			physics.acceleration = QFE::MATH::Vector3::Zero();
		});

	return true;
}

bool QFE::GAMESYSTEM::AutoScrollSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	QFE::EntityManager& entityManager = systems.sceneManager->GetCurrentSceneEntityManager();
	// AutoScrollComponentを持つエンティティに対して処理を行う
	entityManager.Each<QFE::STG::AutoScrollComponent>([&](uint32_t entityId, QFE::STG::AutoScrollComponent& autoScrollComp) {
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			QFE::MATH::EulerTransform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
			if (autoScrollComp.distance > 0.0f) {
				autoScrollComp.distance -= autoScrollComp.speed * deltaTime;
				transform.translate.z += autoScrollComp.speed * deltaTime;
			}
		}
		});

	return true;
}

bool QFE::GAMESYSTEM::ShootingPlayerSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	// マネージャの取得
	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();
	QFE::INPUT::InputInterface* inputInterface = systems.inputInterface.get();

	// シューティングプレイヤーの実行
	entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent& shootingPlayerComp) {
		if (!shootingPlayerComp.inputEnabled) {
			shootingPlayerComp.velocity = QFE::MATH::Vector3::Zero();
			return;
		}
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			QFE::MATH::EulerTransform& playerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
			float speed = shootingPlayerComp.speed;
			float targetRotateZ = 0.0f;
			float rotatePower = 1.0f;

			if (inputInterface->GetKeyPress("Slow")) {
				speed *= 0.5f;
				rotatePower *= 0.5f;
			}

			// プレイヤーの移動処理
			if (inputInterface->GetKeyPress("Up")) {
				shootingPlayerComp.velocity.z = speed * deltaTime;
			}
			if (inputInterface->GetKeyPress("Down")) {
				shootingPlayerComp.velocity.z = -speed * deltaTime;
			}
			if (inputInterface->GetKeyPress("Left")) {

				targetRotateZ = 0.7f;
				shootingPlayerComp.velocity.x = -speed * deltaTime;
			}
			if (inputInterface->GetKeyPress("Right")) {
				targetRotateZ = -0.7f;
				shootingPlayerComp.velocity.x = speed * deltaTime;
			}

			if (inputInterface->GetKeyTrigger("Left")) {
				if (playerTransform.rotate.z < -0.6f) {
					playerTransform.rotate.z = -3.0f;
				}
			}
			if (inputInterface->GetKeyTrigger("Right")) {
				if (playerTransform.rotate.z > 0.6f) {
					playerTransform.rotate.z = 3.0f;
				}
			}

			// プレイヤーの位置を更新
			playerTransform.translate += shootingPlayerComp.velocity;
			shootingPlayerComp.velocity.x *= shootingPlayerComp.damping; // 減衰を適用
			shootingPlayerComp.velocity.z *= shootingPlayerComp.damping; // 減衰を適用

			// プレイヤーの回転処理（Z軸回転）
			playerTransform.rotate.z = QFE::MATH::SimpleEaseIn(playerTransform.rotate.z, targetRotateZ * rotatePower, 0.1f);
			const QFE::MATH::Matrix4x4 playerWorldMatrix =
				QFE::SCENE::GetWorldMatrix(entityManager, entityId);
		}
		});
	return false;
}

bool QFE::GAMESYSTEM::EnemySpawnerSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	// マネージャの取得
	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();

	entityManager.Each<QFE::STG::EntitySpawnerComponent>([&](uint32_t entityId, QFE::STG::EntitySpawnerComponent& enemySpawnerComp) {
		
		// スポーントリガーオブジェクトの位置を取得
		entityManager.Each<QFE::STG::SpawnTriggerComponent>([&](uint32_t triggerEntityId, QFE::STG::SpawnTriggerComponent& spawnerTriggerComp) {
			// マスクの判定
			if ((enemySpawnerComp.spawnMask.value & spawnerTriggerComp.spawnMask.value) == 0) {
				return;
			}
			// トリガーオブジェクトの位置を取得
			float triggerDistance = 9999999.9f;
			if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(triggerEntityId)) {
				QFE::MATH::EulerTransform& triggerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(triggerEntityId).transform;
				// スポーントリガー距離の判定
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::EulerTransform& spawnerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					triggerDistance = (triggerTransform.translate - spawnerTransform.translate).Length();
					if (triggerDistance <= enemySpawnerComp.spawnTriggerDistance) {
						// エンティティをスポーン
						uint32_t spawnedEntityId =
							sceneManager->LoadEntityOnCurrentSceneFromJsonObject(resources.assetDir + enemySpawnerComp.entityPrefabName);
						// スポーンしたエンティティの位置をスポナーの位置に設定
						if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(spawnedEntityId)) {
							QFE::MATH::EulerTransform& spawnedTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(spawnedEntityId).transform;
							spawnedTransform.translate = spawnerTransform.translate;
						}
						// スポーン後、スポナーを削除する
						entityManager.RemoveEntity(entityId);
					}
				}
			}
		});
		});

	return true;
}

bool QFE::GAMESYSTEM::ShootingEnemySystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	// マネージャの取得
	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();

	// 敵の処理
	std::vector<QFE::MATH::Vector3> playerPositionsE;
	entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent& shootingPlayerComp) {
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			playerPositionsE.push_back(GetWorldPosition(entityManager, entityId));
		}
		});
	entityManager.Each<QFE::STG::EnemyAIComponent>([&](uint32_t entityId, QFE::STG::EnemyAIComponent& enemyAIComp) {
		enemyAIComp.shotTimer -= deltaTime;
		if (enemyAIComp.shotTimer <= 0.0f) {
			enemyAIComp.shotTimer = enemyAIComp.shotInterval;
			uint32_t bulletEntityId =
				sceneManager->LoadEntityOnCurrentSceneFromJsonObject(resources.assetDir + enemyAIComp.bulletName);
			const QFE::MATH::Vector3 enemyWorldPosition = GetWorldPosition(entityManager, entityId);
			if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId) &&
				entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
				bulletTransform.translate = enemyWorldPosition;
			}
			// プレイヤーの位置に向かって弾丸を発射する
			if (!playerPositionsE.empty()) {
				QFE::MATH::Vector3 targetPosition = playerPositionsE[0]; // 最初のプレイヤーの位置をターゲットにする
				if (entityManager.HasComponent<QFE::STG::BulletComponent>(bulletEntityId) &&
					entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId)) {
					QFE::STG::BulletComponent& bulletComp = entityManager.GetComponent<QFE::STG::BulletComponent>(bulletEntityId);
					bulletComp.dir = (targetPosition - enemyWorldPosition).Normalize();
				}
			}
		}
		});
	return false;
}

bool QFE::GAMESYSTEM::BulletUpdateSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	// マネージャの取得
	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();

	// 弾丸の処理
	entityManager.Each<QFE::STG::BulletComponent>([&](uint32_t entityId, QFE::STG::BulletComponent& bulletComp) {
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
			bulletTransform.translate += bulletComp.dir * bulletComp.speed * deltaTime;
			bulletComp.lifeTimeMax -= deltaTime;
			if (bulletComp.lifeTimeMax <= 0.0f) {
				entityManager.RemoveEntity(entityId);
			}
		}
		});
	return true;
}

bool QFE::GAMESYSTEM::CollisionSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();
	QFE::FRAMEWORK::BeginCollisionTriggerFrame(entityManager);
	std::vector<QFE::FRAMEWORK::ComponentParameterCommand> parameterCommands;

	// コライダーの処理
	std::vector<uint32_t>colliderEntityIds;
	std::map<uint32_t, QFE::SCENE::SphereColliderComponent> colliderComponents;
	std::map<uint32_t, QFE::SCENE::TransformComponent> transformComponents;
	entityManager.Each<QFE::SCENE::SphereColliderComponent>([&](uint32_t entityId, QFE::SCENE::SphereColliderComponent& colliderComp) {
		if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			return;
		}
		colliderEntityIds.push_back(entityId);
		colliderComponents[entityId] = colliderComp;
		transformComponents[entityId] = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
		});
	// 球のコライダー同士の衝突判定	
	for (size_t i = 0; i < colliderEntityIds.size(); ++i) {
		uint32_t entityIdA = colliderEntityIds[i];
		QFE::SCENE::SphereColliderComponent& colliderA = colliderComponents[entityIdA];
		QFE::MATH::EulerTransform& transformA = transformComponents[entityIdA].transform;
		for (size_t j = i + 1; j < colliderEntityIds.size(); ++j) {
			uint32_t entityIdB = colliderEntityIds[j];
			QFE::SCENE::SphereColliderComponent& colliderB = colliderComponents[entityIdB];
			QFE::MATH::EulerTransform& transformB = transformComponents[entityIdB].transform;

			// タグマスクの判定
			if ((colliderA.mask & colliderB.mask) != 0) {
				continue; // 衝突判定をスキップ
			}

			// 衝突判定
			float distance = (transformA.translate - transformB.translate).Length();
			if (distance < (colliderA.radius + colliderB.radius)) {
				QFE::FRAMEWORK::NotifyCollisionTrigger(
					entityManager, entityIdA, entityIdB, colliderB.mask, parameterCommands);
				QFE::FRAMEWORK::NotifyCollisionTrigger(
					entityManager, entityIdB, entityIdA, colliderA.mask, parameterCommands);
			}
		}
	}
	QFE::FRAMEWORK::EndCollisionTriggerFrame(entityManager, parameterCommands);
	QFE::FRAMEWORK::ApplyComponentParameterCommands(entityManager, parameterCommands);
	return true;
}

bool QFE::GAMESYSTEM::PlayerTrackingSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();

	// プレイヤー自動トラッキング処理
	std::vector<uint32_t> playerEntityIds;
	std::vector<QFE::MATH::Vector3> playerPositions;
	entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent& shootingPlayerComp) {
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			QFE::MATH::EulerTransform& playerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
			playerPositions.push_back(playerTransform.translate);
			playerEntityIds.push_back(entityId);
		}
		});
	entityManager.Each<QFE::STG::PlayerTrackingComponent>([&](uint32_t entityId, QFE::STG::PlayerTrackingComponent& autoTrackComp) {
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			QFE::MATH::EulerTransform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
			if (!playerPositions.empty()) {
				// プレイヤーの平均位置を計算
				QFE::MATH::Vector3 averagePosition = { 0.0f, 0.0f, 0.0f };
				for (const auto& pos : playerPositions) {
					averagePosition += pos;
				}
				averagePosition.x /= static_cast<float>(playerPositions.size());
				averagePosition.y /= static_cast<float>(playerPositions.size());
				averagePosition.z /= static_cast<float>(playerPositions.size());
				// オブジェクトの位置をプレイヤーの平均位置に
				if (autoTrackComp.isTrackingX)
				{
					transform.translate.x = averagePosition.x + autoTrackComp.trackingOffsetPos.x;
				}
				if (autoTrackComp.isTrackingY)
				{
					transform.translate.y = averagePosition.y + autoTrackComp.trackingOffsetPos.y;
				}
				if (autoTrackComp.isTrackingZ)
				{
					transform.translate.z = averagePosition.z + autoTrackComp.trackingOffsetPos.z;
				}
			}
			// 回転のトラッキング
			if (autoTrackComp.isTrackingRotation) {
				// プレイヤーの平均位置を計算
				QFE::MATH::Vector3 averagePosition = { 0.0f, 0.0f, 0.0f };
				for (const auto& pos : playerPositions) {
					averagePosition += pos;
				}
				averagePosition.x /= static_cast<float>(playerPositions.size());
				averagePosition.y /= static_cast<float>(playerPositions.size());
				averagePosition.z /= static_cast<float>(playerPositions.size());
				QFE::MATH::Vector3 directionToPlayer = averagePosition - transform.translate;
				directionToPlayer = directionToPlayer.Normalize();
				float targetYaw = atan2f(directionToPlayer.x, directionToPlayer.z);
				float targetPitch = asinf(-directionToPlayer.y);
				targetYaw = std::lerp(targetYaw, autoTrackComp.trackingRotationOffset.y, autoTrackComp.trackingRotationTranspose);
				targetPitch = std::lerp(targetPitch, autoTrackComp.trackingRotationOffset.x, autoTrackComp.trackingRotationTranspose);

				transform.rotate.x = targetPitch;
			}
		}
		});
	return true;
}

bool QFE::GAMESYSTEM::MoveLimitSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();

	// 移動制限コンポーネントの処理
	entityManager.Each<QFE::STG::MoveLimitComponent>([&](uint32_t entityId, QFE::STG::MoveLimitComponent& moveLimitComp) {
		if (moveLimitComp.autoScrollDistance > 0.0f) {
			float scrollAmount = moveLimitComp.autoScrollSpeed.Length() * deltaTime;
			moveLimitComp.center += moveLimitComp.autoScrollSpeed * deltaTime;
			moveLimitComp.autoScrollDistance -= scrollAmount;
		}

		if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			return;
		}
		QFE::MATH::EulerTransform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
		QFE::MATH::Vector3 minLimit = moveLimitComp.center + moveLimitComp.minLimit;
		QFE::MATH::Vector3 maxLimit = moveLimitComp.center + moveLimitComp.maxLimit;
		transform.translate.x = std::clamp(transform.translate.x, minLimit.x, maxLimit.x);
		transform.translate.y = std::clamp(transform.translate.y, minLimit.y, maxLimit.y);
		transform.translate.z = std::clamp(transform.translate.z, minLimit.z, maxLimit.z);
		});
	return false;
}

bool QFE::GAMESYSTEM::BulletEmitterSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	(void)deltaTime;

	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();

	entityManager.Each<QFE::STG::BulletEmitterComponent>(
		[&](uint32_t entityId, QFE::STG::BulletEmitterComponent& bulletEmitterComp) {
			std::vector<uint32_t> requestedPatterns;
			requestedPatterns.swap(bulletEmitterComp.requestedPatternIndices);
			if (bulletEmitterComp.emitRequest) {
				requestedPatterns.push_back(0);
				bulletEmitterComp.emitRequest = false;
			}
			if (requestedPatterns.empty()) {
				return;
			}

			if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				return;
			}

			const QFE::MATH::Matrix4x4 emitterWorldMatrix =
				QFE::SCENE::GetWorldMatrix(entityManager, entityId);
			QFE::MATH::Vector3 worldAxisX = {
				emitterWorldMatrix.m[0][0], emitterWorldMatrix.m[0][1], emitterWorldMatrix.m[0][2]
			};
			QFE::MATH::Vector3 worldAxisY = {
				emitterWorldMatrix.m[1][0], emitterWorldMatrix.m[1][1], emitterWorldMatrix.m[1][2]
			};
			QFE::MATH::Vector3 worldAxisZ = {
				emitterWorldMatrix.m[2][0], emitterWorldMatrix.m[2][1], emitterWorldMatrix.m[2][2]
			};
			worldAxisX = worldAxisX.Normalize();
			worldAxisY = worldAxisY.Normalize();
			worldAxisZ = worldAxisZ.Normalize();
			const QFE::MATH::Vector3 emitterWorldPosition = {
				emitterWorldMatrix.m[3][0], emitterWorldMatrix.m[3][1], emitterWorldMatrix.m[3][2]
			};

			for (const uint32_t patternIndex : requestedPatterns) {
				if (patternIndex >= bulletEmitterComp.patterns.size()) {
					continue;
				}
				const QFE::STG::BulletEmitterPattern& pattern = bulletEmitterComp.patterns[patternIndex];
				if (pattern.emitBulletName.empty() || pattern.emitCount == 0) {
					continue;
				}

				// エミッターや親の拡縮率は、発射半径とオフセットへ影響させない。
				const QFE::MATH::Vector3 sphereCenter = emitterWorldPosition +
					worldAxisX * pattern.emitPos.x +
					worldAxisY * pattern.emitPos.y +
					worldAxisZ * pattern.emitPos.z;

				QFE::MATH::Vector3 localBaseDirection = pattern.emitDir;
				if (localBaseDirection.LengthSq() == 0.0f) {
					localBaseDirection = { 0.0f, 0.0f, 1.0f };
				}
				QFE::MATH::Vector3 worldBaseDirection =
					(worldAxisX * localBaseDirection.x +
					 worldAxisY * localBaseDirection.y +
					 worldAxisZ * localBaseDirection.z).Normalize();
				if (worldBaseDirection.LengthSq() == 0.0f) {
					worldBaseDirection = { 0.0f, 0.0f, 1.0f };
				}

				const QFE::MATH::Vector3 baseSpherical =
					QFE::MATH::Vector3::CartesianToSpherical(worldBaseDirection);
				const float radius = pattern.emitRadius > 0.0f ? pattern.emitRadius : 0.0f;

				for (uint32_t bulletIndex = 0; bulletIndex < pattern.emitCount; ++bulletIndex) {
					// 球座標は x=半径、y=極角theta、z=方位角phi。
					const QFE::MATH::Vector3 bulletSpherical = {
						1.0f,
						baseSpherical.y + pattern.bulletAngleX * static_cast<float>(bulletIndex),
						baseSpherical.z + pattern.bulletAngleY * static_cast<float>(bulletIndex)
					};
					const QFE::MATH::Vector3 bulletDirection =
						QFE::MATH::Vector3::SphericalToCartesian(bulletSpherical).Normalize();
					const QFE::MATH::Vector3 bulletPosition = sphereCenter + bulletDirection * radius;

					const uint32_t bulletEntityId = sceneManager->LoadEntityOnCurrentSceneFromJsonObject(
						resources.assetDir + pattern.emitBulletName);
					if (bulletEntityId == UINT32_MAX) {
						continue;
					}

					if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId)) {
						QFE::MATH::EulerTransform& bulletTransform =
							entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
						bulletTransform.translate = bulletPosition;
						bulletTransform.rotate = QFE::MATH::Vector3::LookAt(
							bulletPosition, bulletPosition + bulletDirection);
					}
					if (entityManager.HasComponent<QFE::STG::BulletComponent>(bulletEntityId)) {
						entityManager.GetComponent<QFE::STG::BulletComponent>(bulletEntityId).dir = bulletDirection;
					}
				}
			}
		});

	return false;
}

bool QFE::GAMESYSTEM::BulletEmitterTriggerSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	(void)resources;

	QFE::EntityManager& entityManager =
		systems.sceneManager->GetCurrentSceneEntityManager();
	QFE::INPUT::InputInterface* inputInterface = systems.inputInterface.get();

	entityManager.Each<QFE::STG::InputBulletEmitterTriggerComponent>(
		[&](uint32_t entityId, QFE::STG::InputBulletEmitterTriggerComponent& triggerComp) {
			if (entityManager.HasComponent<QFE::STG::ShootingPlayerComponent>(entityId) &&
				!entityManager.GetComponent<QFE::STG::ShootingPlayerComponent>(entityId).inputEnabled) {
				for (QFE::STG::InputBulletEmitterTriggerSetting& trigger : triggerComp.triggers) {
					trigger.repeatTimer = 0.0f;
				}
				return;
			}

			if (!entityManager.HasComponent<QFE::STG::BulletEmitterComponent>(entityId)) {
				for (QFE::STG::InputBulletEmitterTriggerSetting& trigger : triggerComp.triggers) {
					trigger.repeatTimer = 0.0f;
				}
				return;
			}

			for (QFE::STG::InputBulletEmitterTriggerSetting& trigger : triggerComp.triggers) {
				if (!trigger.enabled) {
					trigger.repeatTimer = 0.0f;
					continue;
				}

				auto getInput = [&](uint32_t triggerMode) {
					bool active = false;
					if (!trigger.inputActionName.empty()) {
						switch (triggerMode) {
						case QFE::STG::BulletEmitterInputTrigger:
							active |= inputInterface->GetKeyTrigger(trigger.inputActionName);
							break;
						case QFE::STG::BulletEmitterInputRelease:
							active |= inputInterface->GetKeyRelease(trigger.inputActionName);
							break;
						default:
							active |= inputInterface->GetKeyPress(trigger.inputActionName);
							break;
						}
					}
					if (trigger.mouseButton >= 0 && trigger.mouseButton <= INT8_MAX) {
						const int8_t mouseButton = static_cast<int8_t>(trigger.mouseButton);
						switch (triggerMode) {
						case QFE::STG::BulletEmitterInputTrigger:
							active |= inputInterface->GetMouseTrigger(mouseButton);
							break;
						case QFE::STG::BulletEmitterInputRelease:
							active |= inputInterface->GetMouseRelease(mouseButton);
							break;
						default:
							active |= inputInterface->GetMousePress(mouseButton);
							break;
						}
					}
					if (trigger.gamePadButton != 0 && trigger.gamePadButton <= UINT16_MAX) {
						const uint16_t gamePadButton = static_cast<uint16_t>(trigger.gamePadButton);
						switch (triggerMode) {
						case QFE::STG::BulletEmitterInputTrigger:
							active |= inputInterface->GetGamePadTrigger(gamePadButton);
							break;
						case QFE::STG::BulletEmitterInputRelease:
							active |= inputInterface->GetGamePadRelease(gamePadButton);
							break;
						default:
							active |= inputInterface->GetGamePadPress(gamePadButton);
							break;
						}
					}
					return active;
				};

				const uint32_t triggerMode = trigger.triggerMode;
				const bool inputActive = getInput(triggerMode);
				bool requestEmit = false;
				if (triggerMode == QFE::STG::BulletEmitterInputPress) {
					if (!inputActive) {
						trigger.repeatTimer = 0.0f;
						continue;
					}
					trigger.repeatTimer -= deltaTime;
					if (trigger.repeatTimer <= 0.0f) {
						requestEmit = true;
						trigger.repeatTimer = trigger.repeatInterval > 0.0f
							? trigger.repeatInterval
							: 0.0f;
					}
				} else {
					requestEmit = inputActive;
				}

				if (requestEmit) {
					RequestBulletEmitterPattern(entityManager, entityId, trigger.patternIndex);
				}
			}
		});

	entityManager.Each<QFE::STG::PeriodicBulletEmitterTriggerComponent>(
		[&](uint32_t entityId, QFE::STG::PeriodicBulletEmitterTriggerComponent& triggerComp) {
			if (!entityManager.HasComponent<QFE::STG::BulletEmitterComponent>(entityId)) {
				for (QFE::STG::PeriodicBulletEmitterTriggerSetting& trigger : triggerComp.triggers) {
					trigger.initialized = false;
					trigger.remainingTime = 0.0f;
				}
				return;
			}

			for (QFE::STG::PeriodicBulletEmitterTriggerSetting& trigger : triggerComp.triggers) {
				if (!trigger.enabled) {
					trigger.initialized = false;
					trigger.remainingTime = 0.0f;
					continue;
				}

				if (!trigger.initialized) {
					trigger.initialized = true;
					trigger.remainingTime = trigger.emitOnStart ? 0.0f : trigger.interval;
				}

				trigger.remainingTime -= deltaTime;
				if (trigger.remainingTime <= 0.0f) {
					RequestBulletEmitterPattern(entityManager, entityId, trigger.patternIndex);
					trigger.remainingTime = trigger.interval > 0.0f ? trigger.interval : 0.0f;
				}
			}
		});

	return true;
}
