#include "ShootingSystem.h"

#include "scene/SceneManager.h"
#include "components/AllComponent.h"

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
					playerTransform.rotate.z = -5.0f;
				}
			}
			if (inputInterface->GetKeyTrigger("Right")) {
				if (playerTransform.rotate.z > 0.6f) {
					playerTransform.rotate.z = 5.0f;
				}
			}

			// プレイヤーの位置を更新
			playerTransform.translate += shootingPlayerComp.velocity;
			shootingPlayerComp.velocity.x *= shootingPlayerComp.damping; // 減衰を適用
			shootingPlayerComp.velocity.z *= shootingPlayerComp.damping; // 減衰を適用

			// プレイヤーの回転処理（Z軸回転）
			playerTransform.rotate.z = QFE::MATH::SimpleEaseIn(playerTransform.rotate.z, targetRotateZ * rotatePower, 0.1f);

			// プレイヤーの射撃処理
			if (shootingPlayerComp.shootTimer > 0.0f) {
				shootingPlayerComp.shootTimer -= deltaTime;
			} else {
				shootingPlayerComp.shootTimer = 0.0f;
			}
			if (shootingPlayerComp.bombTimer > 0.0f) {
				shootingPlayerComp.bombTimer -= deltaTime;
			} else {
				shootingPlayerComp.bombTimer = 0.0f;
			}

			if (inputInterface->GetKeyPress("Shot")) {
				if (shootingPlayerComp.shootTimer <= 0.0f) {
					uint32_t bulletEntityId =
						sceneManager->LoadEntityOnCurrentSceneFromJsonObject(resources.assetDir + shootingPlayerComp.bulletPrefabName);

					if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId)) {
						QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
						bulletTransform.translate = playerTransform.translate + shootingPlayerComp.bulletSpawnOffset;
					}
					shootingPlayerComp.shootTimer = shootingPlayerComp.shootInterval;
				}
			}
			// プレイヤーのボム処理
			if (inputInterface->GetKeyRelease("Shot")) {
				if(shootingPlayerComp.bombTimer <= 0.0f) {
					shootingPlayerComp.bombTimer = shootingPlayerComp.bombInterval;

					uint32_t bombEntityId =
						sceneManager->LoadEntityOnCurrentSceneFromJsonObject(resources.assetDir + shootingPlayerComp.bombPrefabName);

					if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bombEntityId)) {
						QFE::MATH::EulerTransform& bombTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bombEntityId).transform;
						bombTransform.translate = playerTransform.translate + shootingPlayerComp.bombSpawnOffset;
					}
				}
			}
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
			QFE::MATH::EulerTransform& playerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
			playerPositionsE.push_back(playerTransform.translate);
		}
		});
	entityManager.Each<QFE::STG::EnemyAIComponent>([&](uint32_t entityId, QFE::STG::EnemyAIComponent& enemyAIComp) {
		enemyAIComp.shotTimer -= deltaTime;
		if (enemyAIComp.shotTimer <= 0.0f) {
			enemyAIComp.shotTimer = enemyAIComp.shotInterval;
			uint32_t bulletEntityId =
				sceneManager->LoadEntityOnCurrentSceneFromJsonObject(resources.assetDir + enemyAIComp.bulletName);
			if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId) &&
				entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
				QFE::MATH::EulerTransform& enemyTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
				bulletTransform.translate = enemyTransform.translate;
			}
			// プレイヤーの位置に向かって弾丸を発射する
			if (!playerPositionsE.empty()) {
				QFE::MATH::Vector3 targetPosition = playerPositionsE[0]; // 最初のプレイヤーの位置をターゲットにする
				if (entityManager.HasComponent<QFE::STG::BulletComponent>(bulletEntityId) &&
					entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId)) {
					QFE::STG::BulletComponent& bulletComp = entityManager.GetComponent<QFE::STG::BulletComponent>(bulletEntityId);
					QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
					bulletComp.dir = (targetPosition - bulletTransform.translate).Normalize();
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
				// 衝突が発生した場合の処理
				if (entityManager.HasComponent<QFE::STG::HealthComponent>(entityIdA)) {
					QFE::STG::HealthComponent& healthCompA = entityManager.GetComponent<QFE::STG::HealthComponent>(entityIdA);
					healthCompA.health -= 1;
					if (healthCompA.health <= 0) {
						entityManager.RemoveEntity(entityIdA);
					}
				} else {
					entityManager.RemoveEntity(entityIdA);
				}
				if (entityManager.HasComponent<QFE::STG::HealthComponent>(entityIdB)) {
					QFE::STG::HealthComponent& healthCompB = entityManager.GetComponent<QFE::STG::HealthComponent>(entityIdB);
					healthCompB.health -= 1;
					if (healthCompB.health <= 0) {
						entityManager.RemoveEntity(entityIdB);
					}
				} else {
					entityManager.RemoveEntity(entityIdB);
				}
			}
		}
	}
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
