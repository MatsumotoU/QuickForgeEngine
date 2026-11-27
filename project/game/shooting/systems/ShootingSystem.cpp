#include "ShootingSystem.h"
#include "CollisionShapes.h"

#include "framework/scene/SceneCoreFramework.h"
#include "framework/input/InputFrameWork.h"
#include "components/AllComponent.h"
#include "game/shooting/components/AllShootingComponents.h"
#include "components/TransformHierarchy.h"
#include "framework/scene/CollisionTriggerSystem.h"
#include "framework/assets/AssetsCoreFramework.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
	// 入力移動するコライダーを静止した箱に対して止める。トリガーは移動を妨げない。
	QFE::MATH::Vector3 MoveInputCollider(QFE::EntityManager& entities, uint32_t id,
		const QFE::MATH::Vector3& movement, QFE::MATH::Vector3* velocity = nullptr) {
		using namespace QFE::GAMESYSTEM::CollisionShapes;
		auto* transform = entities.GetComponentPtr<QFE::SCENE::TransformComponent>(id);
		if (!transform) return movement;
		Shape moving{};
		if (const auto* sphere = entities.GetComponentPtr<QFE::SCENE::SphereColliderComponent>(id)) {
			if (sphere->radius <= 0.0f) return movement;
			moving = { id, sphere->mask, true, transform->transform.translate, sphere->radius, {}, {} };
		} else if (const auto* box = entities.GetComponentPtr<QFE::SCENE::AABBColliderComponent>(id)) {
			moving = MakeBox(id, box->mask, transform->transform, box->halfExtents, false);
		} else if (const auto* orientedBox = entities.GetComponentPtr<QFE::SCENE::OBBColliderComponent>(id)) {
			moving = MakeBox(id, orientedBox->mask, transform->transform, orientedBox->halfExtents, true);
		} else return movement;
		std::vector<Shape> walls;
		const auto addWall = [&](uint32_t wallId, uint32_t mask, const QFE::MATH::Vector3& extents, bool oriented) {
			if (wallId == id || (moving.mask & mask) != 0 ||
				entities.HasComponent<QFE::SCENE::CollisionTriggerComponent>(wallId) ||
				entities.HasComponent<QFE::COMPONENTS::InputMovementComponent>(wallId)) return;
			const auto* physics = entities.GetComponentPtr<QFE::COMPONENTS::PhysicsComponent>(wallId);
			if (physics && physics->mass > 0.0f) return;
			const auto* wallTransform = entities.GetComponentPtr<QFE::SCENE::TransformComponent>(wallId);
			if (wallTransform) walls.push_back(MakeBox(wallId, mask, wallTransform->transform, extents, oriented));
		};
		entities.Each<QFE::SCENE::AABBColliderComponent>([&](uint32_t wallId, QFE::SCENE::AABBColliderComponent& wall) {
			addWall(wallId, wall.mask, wall.halfExtents, false);
		});
		entities.Each<QFE::SCENE::OBBColliderComponent>([&](uint32_t wallId, QFE::SCENE::OBBColliderComponent& wall) {
			addWall(wallId, wall.mask, wall.halfExtents, true);
		});
		QFE::MATH::Vector3 position = transform->transform.translate;
		QFE::MATH::Vector3 remaining = movement;
		if (!moving.sphere) {
			// 回転した OBB の角が壁に入った場合も、掃引する前に壁の外へ戻す。
			for (int iteration = 0; iteration < 8; ++iteration) {
				bool corrected = false;
				for (const auto& wall : walls) {
					moving.center = position;
					float depth = 0.0f;
					QFE::MATH::Vector3 normal{};
					if (!BoxPenetration(moving, wall, depth, normal)) continue;
					position += normal * (depth + 1.0e-5f);
					if (Dot(remaining, normal) < 0.0f) remaining = remaining - normal * Dot(remaining, normal);
					if (velocity && Dot(*velocity, normal) < 0.0f) *velocity = *velocity - normal * Dot(*velocity, normal);
					corrected = true;
				}
				if (!corrected) break;
			}
		}
		// 複数の壁が交わる場所でも、それぞれの壁に向かう成分を取り除く。
		for (int iteration = 0; iteration < 4; ++iteration) {
			moving.center = position;
			float firstTime = 1.0f;
			QFE::MATH::Vector3 normal{};
			bool hit = false;
			for (const auto& wall : walls) {
				float time;
				QFE::MATH::Vector3 candidateNormal{};
				const bool hitWall = moving.sphere
					? SweepSphereBox(position, moving.radius, remaining, wall, time, candidateNormal)
					: SweepBoxBox(moving, remaining, wall, time, candidateNormal);
				if (hitWall && time <= firstTime) {
					firstTime = time; normal = candidateNormal; hit = true;
				}
			}
			position += remaining * firstTime;
			if (!hit) break;
			remaining = remaining * (1.0f - firstTime);
			remaining = remaining - normal * Dot(remaining, normal);
			if (velocity && Dot(*velocity, normal) < 0.0f) *velocity = *velocity - normal * Dot(*velocity, normal);
		}
		return position - transform->transform.translate;
	}

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

	void RequestBulletEmitterPattern(
		QFE::EntityManager& entityManager,
		uint32_t entityId,
		uint32_t patternIndex) {
		if (entityManager.HasComponent<QFE::STG::BulletEmitterComponent>(entityId)) {
			entityManager.GetComponent<QFE::STG::BulletEmitterComponent>(entityId)
				.RequestEmit(patternIndex);
		}
	}

	void ApplyGameCollisionAction(QFE::EntityManager& entities,
		const QFE::FRAMEWORK::CollisionTriggerCommand& command) {
		const auto& action = command.action;
		const std::string type = action.value("type", std::string{});
		if (type != "IncreaseHealth" && type != "DecreaseHealth") return;
		uint32_t target = command.selfEntityId;
		const auto targetType = QFE::FRAMEWORK::ParameterTargetFromString(action.value("target", std::string("Self")));
		if (targetType == QFE::FRAMEWORK::ParameterTarget::Other) target = command.otherEntityId;
		else if (targetType == QFE::FRAMEWORK::ParameterTarget::EntityReference &&
			!QFE::SCENE::TryGetEntityIdByUuid(entities, action.value("targetUuid", std::string{}), target)) return;
		if (!entities.IsActiveEntity(target)) return;
		auto* status = entities.GetComponentPtr<QFE::STG::StatusComponent>(target);
		if (!status) return;
		uint32_t amount = 0;
		if (action.value("amountSource", std::string("Fixed")) == "OtherAttackPower") {
			// 接触相手の攻撃力を使用し、相手がいない・ステータスを持たない場合は何もしない。
			if (!entities.IsActiveEntity(command.otherEntityId)) return;
			const auto* other = entities.GetComponentPtr<QFE::STG::StatusComponent>(command.otherEntityId);
			if (!other) return;
			amount = other->attackPower;
		} else if (action.contains("amount") && action["amount"].is_number_integer()) {
			const int64_t value = action["amount"].get<int64_t>();
			if (value < 0 || static_cast<uint64_t>(value) > UINT32_MAX) return;
			amount = static_cast<uint32_t>(value);
		}
		if (type == "DecreaseHealth") status->health = amount >= status->health ? 0 : status->health - amount;
		else {
			const uint64_t increased = static_cast<uint64_t>(status->health) + amount;
			status->health = static_cast<uint32_t>((std::min)(increased, static_cast<uint64_t>(UINT32_MAX)));
		}
	}
}

void QFE::GAMESYSTEM::RemoveDefeatedEnemies(QFE::EntityManager& entityManager) {
	std::vector<uint32_t> defeated;
	entityManager.Each<QFE::STG::StatusComponent>([&](uint32_t id, QFE::STG::StatusComponent& status) {
		if (status.health != 0) return;
		const auto* info = entityManager.GetComponentPtr<QFE::SCENE::ObjectInfoComponent>(id);
		// ステータスはプレイヤーにも使うため、敵タグがあるエンティティだけを消す。
		if (info && std::find(info->tags.begin(), info->tags.end(), "Enemy") != info->tags.end())
			defeated.push_back(id);
	});
	// 列挙中の削除を避け、既存のフレーム終了時の削除処理に委ねる。
	for (const uint32_t id : defeated) entityManager.RemoveEntity(id);
}

bool QFE::GAMESYSTEM::EncounterRoomSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	auto& entities = QFE::FRAMEWORK::GetSceneEntities(*systems.sceneManager);
	std::vector<uint32_t> roomIds;
	entities.Each<QFE::STG::EncounterRoomComponent>([&](uint32_t id, QFE::STG::EncounterRoomComponent&) {
		roomIds.push_back(id);
	});
	// 生成中はコンポーネントの格納領域が変わり得るため、部屋の列挙を先に終える。
	for (const uint32_t roomId : roomIds) {
		auto* room = entities.GetComponentPtr<QFE::STG::EncounterRoomComponent>(roomId);
		if (!room || !room->enabled) continue;
		const auto* roomTransform = entities.GetComponentPtr<QFE::SCENE::TransformComponent>(roomId);
		uint32_t playerId = UINT32_MAX;
		if (!roomTransform || !QFE::SCENE::TryGetEntityIdByUuid(entities, room->player.uuid, playerId) ||
			!entities.HasComponent<QFE::SCENE::TransformComponent>(playerId)) continue;
		const auto center = GetWorldPosition(entities, roomId);

		if (!room->initialized) {
			std::vector<uint32_t> doors;
			std::vector<QFE::MATH::Vector3> positions;
			std::vector<uint32_t> enemies;
			bool valid = true;
			for (const auto& door : room->doors) {
				uint32_t id = UINT32_MAX;
				if (!QFE::SCENE::TryGetEntityIdByUuid(entities, door.entity.uuid, id) ||
					!entities.HasComponent<QFE::SCENE::TransformComponent>(id)) { valid = false; break; }
				doors.push_back(id);
				positions.push_back(entities.GetComponent<QFE::SCENE::TransformComponent>(id).transform.translate);
			}
			for (const auto& enemy : room->enemies) {
				uint32_t id = UINT32_MAX;
				if (!QFE::SCENE::TryGetEntityIdByUuid(entities, enemy.entity.uuid, id)) { valid = false; break; }
				enemies.push_back(id);
			}
			if (!valid) continue;
			room->doorIds = std::move(doors);
			room->closedDoorPositions = std::move(positions);
			room->enemyIds = std::move(enemies);
			room->initialized = true;
			// エディターで配置した閉位置を基準に、開始前は通れる位置へ移す。
			for (size_t i = 0; i < room->doorIds.size(); ++i) {
				entities.GetComponent<QFE::SCENE::TransformComponent>(room->doorIds[i]).transform.translate =
					room->closedDoorPositions[i] + room->doors[i].openOffset;
			}
		}
		if (room->cleared) continue;
		if (!room->started) {
			const auto relative = GetWorldPosition(entities, playerId) - center;
			if (std::abs(relative.x) > std::abs(room->halfExtents.x) ||
				std::abs(relative.y) > std::abs(room->halfExtents.y) ||
				std::abs(relative.z) > std::abs(room->halfExtents.z)) continue;
			room->started = true;
			room->doorCloseElapsed = 0.0f;
			const auto spawns = room->spawns;
			for (const auto& spawn : spawns) {
				const std::filesystem::path relativePath(std::u8string(
					reinterpret_cast<const char8_t*>(spawn.entityPath.data()), spawn.entityPath.size()));
				const std::filesystem::path base(std::u8string(
					reinterpret_cast<const char8_t*>(resources.assetDir.data()), resources.assetDir.size()));
				const auto path = base / relativePath;
				uint32_t spawnedId = UINT32_MAX;
				if (!relativePath.empty() && !relativePath.is_absolute() &&
					QFE::FRAMEWORK::ASSETS::IsPathWithin(path, base)) {
					const auto utf8 = path.generic_u8string();
					spawnedId = QFE::FRAMEWORK::LoadSceneEntity(*systems.sceneManager,
						std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size()));
				}
				room = entities.GetComponentPtr<QFE::STG::EncounterRoomComponent>(roomId);
				if (!room) break;
				if (spawnedId == UINT32_MAX) {
					// 生成失敗を全滅と誤認して扉を開かない。
					room->spawnFailed = true;
					QFE_LOG("Encounter room enemy spawn failed: " + spawn.entityPath);
					continue;
				}
				room->enemyIds.push_back(spawnedId);
				if (auto* transform = entities.GetComponentPtr<QFE::SCENE::TransformComponent>(spawnedId))
					transform->transform.translate = center + spawn.offset;
			}
		}
		if (!room) continue;
		// 経過時間から毎回計算し、フレームレートに依存せず終点で止める。
		room->doorCloseElapsed += (std::max)(deltaTime, 0.0f);
		const float progress = room->doorCloseDuration > 0.0f
			? std::clamp(room->doorCloseElapsed / room->doorCloseDuration, 0.0f, 1.0f) : 1.0f;
		const float eased = QFE::MATH::EaseInOut(0.0f, 1.0f, progress);
		for (size_t i = 0; i < room->doorIds.size(); ++i) {
			if (auto* door = entities.GetComponentPtr<QFE::SCENE::TransformComponent>(room->doorIds[i]))
				door->transform.translate = room->closedDoorPositions[i] + room->doors[i].openOffset * (1.0f - eased);
		}
		if (room->spawnFailed) continue;
		const bool hasEnemy = std::any_of(room->enemyIds.begin(), room->enemyIds.end(),
			[&](uint32_t id) { return entities.IsActiveEntity(id); });
		if (!hasEnemy) {
			room->cleared = true;
			for (size_t i = 0; i < room->doorIds.size(); ++i) {
				if (auto* door = entities.GetComponentPtr<QFE::SCENE::TransformComponent>(room->doorIds[i]))
					door->transform.translate = room->closedDoorPositions[i] + room->doors[i].openOffset;
			}
		}
	}
	return true;
}

bool QFE::GAMESYSTEM::InputMovementSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {

	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*systems.sceneManager);
	QFE::INPUT::InputInterface* inputInterface = systems.inputInterface.get();
	if (inputInterface == nullptr || deltaTime <= 0.0f) {
		return false;
	}

	entityManager.Each<QFE::COMPONENTS::InputMovementComponent>(
		[&](uint32_t entityId, QFE::COMPONENTS::InputMovementComponent& movement) {
			if (!movement.enabled ||
				(entityManager.HasComponent<QFE::STG::ShootingPlayerComponent>(entityId) &&
					!entityManager.GetComponent<QFE::STG::ShootingPlayerComponent>(entityId).inputEnabled)) {
				return;
			}

			float horizontalInput = 0.0f;
			float verticalInput = 0.0f;
			const auto allowed = [&](const std::string& action) {
				const bool configured = action.empty() || std::any_of(resources.inputActions.begin(), resources.inputActions.end(),
					[&](const auto& item) { return item.name == action; });
				return configured && QFE::FRAMEWORK::IsGameActionEnabled(*systems.sceneManager, action);
			};
			const auto pressed = [&](const std::string& action) { return allowed(action) && QFE::FRAMEWORK::GetActionPress(inputInterface, resources.inputActions, action); };
			if (pressed(movement.rightActionName)) {
				horizontalInput += 1.0f;
			}
			if (pressed(movement.leftActionName)) {
				horizontalInput -= 1.0f;
			}
			if (pressed(movement.forwardActionName)) {
				verticalInput += 1.0f;
			}
			if (pressed(movement.backwardActionName)) {
				verticalInput -= 1.0f;
			}
			if (movement.useGamePadLeftStick) {
				const QFE::MATH::Vector2 stickInput = QFE::FRAMEWORK::GetGamePadLeftStickDir(inputInterface);
				// スティックも対応する方向アクションの禁止を守る。
				if (stickInput.x > 0.0f && allowed(movement.rightActionName)) horizontalInput += stickInput.x;
				if (stickInput.x < 0.0f && allowed(movement.leftActionName)) horizontalInput += stickInput.x;
				if (stickInput.y > 0.0f && allowed(movement.forwardActionName)) verticalInput += stickInput.y;
				if (stickInput.y < 0.0f && allowed(movement.backwardActionName)) verticalInput += stickInput.y;
			}

			horizontalInput = std::clamp(horizontalInput, -1.0f, 1.0f);
			verticalInput = std::clamp(verticalInput, -1.0f, 1.0f);
			QFE::MATH::Vector3 direction =
				movement.horizontalAxis * horizontalInput + movement.verticalAxis * verticalInput;

			if (!movement.alignRotationToMovement) {
				movement.rotatingToTarget = false;
			} else if (auto* transform = entityManager.GetComponentPtr<QFE::SCENE::TransformComponent>(entityId)) {
				if (direction.LengthSq() > 0.0f) {
					// 移動軸と位置は同じ親ローカル座標なので、前方をその入力方向に合わせる。
					movement.activeTargetRotation = QFE::MATH::Vector3::LookAt(QFE::MATH::Vector3::Zero(), direction);
					movement.rotatingToTarget = true;
				}
				if (movement.rotatingToTarget) {
					// 各軸を最短方向に進め、フレーム時間に応じて速度を一定にする。
					const auto advance = [&](float& current, float target) {
						constexpr float fullTurn = 6.28318530717958647692f;
						const float difference = std::remainder(target - current, fullTurn);
						const float step = movement.rotationSpeed * deltaTime;
						if (movement.rotationSpeed <= 0.0f || std::abs(difference) <= step) {
							current = target;
							return true;
						}
						current += std::copysign(step, difference);
						return false;
					};
					auto& rotation = transform->transform.rotate;
					const bool xDone = advance(rotation.x, movement.activeTargetRotation.x);
					const bool yDone = advance(rotation.y, movement.activeTargetRotation.y);
					const bool zDone = advance(rotation.z, movement.activeTargetRotation.z);
					movement.rotatingToTarget = !(xDone && yDone && zDone);
				}
			}

			if (movement.amount == 0.0f || direction.LengthSq() == 0.0f) {
				const auto* physics = entityManager.GetComponentPtr<QFE::COMPONENTS::PhysicsComponent>(entityId);
				if (!physics || physics->mass <= 0.0f) {
					if (auto* transform = entityManager.GetComponentPtr<QFE::SCENE::TransformComponent>(entityId)) {
						// 入力を離した後も回転が続くため、位置を動かさないフレームでも重なりを解消する。
						transform->transform.translate += MoveInputCollider(entityManager, entityId, QFE::MATH::Vector3::Zero());
					}
				}
				return;
			}
			if (movement.normalizeInput && direction.LengthSq() > 1.0f) {
				direction = direction.Normalize();
			}

			// 物理がある場合は直接位置を変更せず、速度に慣性を残す。
			if (auto* physics = entityManager.GetComponentPtr<QFE::COMPONENTS::PhysicsComponent>(entityId)) {
				if (physics->mass > 0.0f) {
					physics->acceleration += direction * (movement.amount / physics->mass);
				}
				return;
			}

			if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				const auto displacement = direction * movement.amount * deltaTime;
				entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform.translate +=
					MoveInputCollider(entityManager, entityId, displacement);
			}
		});

	return true;
}

bool QFE::GAMESYSTEM::PhysicsComponentSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	(void)resources;

	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*systems.sceneManager);
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
			const auto displacement = physics.velocity * deltaTime;
			// 力による入力移動も同じ壁判定を通し、壁に向かう速度を残さない。
			entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform.translate +=
				entityManager.HasComponent<QFE::COMPONENTS::InputMovementComponent>(entityId)
					? MoveInputCollider(entityManager, entityId, displacement, &physics.velocity) : displacement;
			physics.acceleration = QFE::MATH::Vector3::Zero();
		});

	return true;
}

bool QFE::GAMESYSTEM::AutoScrollSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	(void)resources;

	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*systems.sceneManager);
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
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);
	QFE::INPUT::InputInterface* inputInterface = systems.inputInterface.get();
	const auto pressed = [&](const std::string& action) {
		return QFE::FRAMEWORK::IsGameActionEnabled(*sceneManager, action) &&
			QFE::FRAMEWORK::GetActionPress(inputInterface, resources.inputActions, action);
	};
	const auto triggered = [&](const std::string& action) {
		return QFE::FRAMEWORK::IsGameActionEnabled(*sceneManager, action) &&
			QFE::FRAMEWORK::GetActionTrigger(inputInterface, resources.inputActions, action);
	};

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

			if (pressed("Slow")) {
				speed *= 0.5f;
				rotatePower *= 0.5f;
			}

			// プレイヤーの移動処理
			if (pressed("Up")) {
				shootingPlayerComp.velocity.z = speed * deltaTime;
			}
			if (pressed("Down")) {
				shootingPlayerComp.velocity.z = -speed * deltaTime;
			}
			if (pressed("Left")) {

				targetRotateZ = 0.7f;
				shootingPlayerComp.velocity.x = -speed * deltaTime;
			}
			if (pressed("Right")) {
				targetRotateZ = -0.7f;
				shootingPlayerComp.velocity.x = speed * deltaTime;
			}

			if (triggered("Left")) {
				if (playerTransform.rotate.z < -0.6f) {
					playerTransform.rotate.z = -3.0f;
				}
			}
			if (triggered("Right")) {
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
	(void)resources;
	(void)deltaTime;

	// マネージャの取得
	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);

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
							QFE::FRAMEWORK::LoadSceneEntity(*sceneManager, resources.assetDir + enemySpawnerComp.entityPrefabName);
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
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);

	// 敵の処理
	std::vector<QFE::MATH::Vector3> playerPositionsE;
	entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent&) {
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
			playerPositionsE.push_back(GetWorldPosition(entityManager, entityId));
		}
		});
	entityManager.Each<QFE::STG::EnemyAIComponent>([&](uint32_t entityId, QFE::STG::EnemyAIComponent& enemyAIComp) {
		enemyAIComp.shotTimer -= deltaTime;
		if (enemyAIComp.shotTimer <= 0.0f) {
			enemyAIComp.shotTimer = enemyAIComp.shotInterval;
			uint32_t bulletEntityId =
				QFE::FRAMEWORK::LoadSceneEntity(*sceneManager, resources.assetDir + enemyAIComp.bulletName);
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
	(void)resources;

	// マネージャの取得
	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);

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
	(void)deltaTime;

	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);
	QFE::FRAMEWORK::BeginCollisionTriggerFrame(entityManager);
	std::vector<QFE::FRAMEWORK::CollisionTriggerCommand> parameterCommands;

	using namespace CollisionShapes;
	std::vector<Shape> colliders;
	entityManager.Each<QFE::SCENE::SphereColliderComponent>([&](uint32_t id, QFE::SCENE::SphereColliderComponent& collider) {
		if (auto* transform = entityManager.GetComponentPtr<QFE::SCENE::TransformComponent>(id)) {
			colliders.push_back({ id, collider.mask, true, transform->transform.translate, collider.radius, {}, {} });
		}
	});
	entityManager.Each<QFE::SCENE::AABBColliderComponent>([&](uint32_t id, QFE::SCENE::AABBColliderComponent& collider) {
		if (auto* transform = entityManager.GetComponentPtr<QFE::SCENE::TransformComponent>(id)) {
			colliders.push_back(MakeBox(id, collider.mask, transform->transform, collider.halfExtents, false));
		}
	});
	entityManager.Each<QFE::SCENE::OBBColliderComponent>([&](uint32_t id, QFE::SCENE::OBBColliderComponent& collider) {
		if (auto* transform = entityManager.GetComponentPtr<QFE::SCENE::TransformComponent>(id)) {
			colliders.push_back(MakeBox(id, collider.mask, transform->transform, collider.halfExtents, true));
		}
	});

	// Collect contacts per entity pair so multiple shapes do not emit duplicate Stay actions.
	std::map<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> contacts;
	for (size_t i = 0; i < colliders.size(); ++i) {
		for (size_t j = i + 1; j < colliders.size(); ++j) {
			const Shape& a = colliders[i];
			const Shape& b = colliders[j];
			if (a.entityId == b.entityId || (a.mask & b.mask) != 0 || !Intersects(a, b)) continue;
			const bool ordered = a.entityId < b.entityId;
			auto& masks = contacts[ordered ? std::make_pair(a.entityId, b.entityId) : std::make_pair(b.entityId, a.entityId)];
			masks.first |= ordered ? a.mask : b.mask;
			masks.second |= ordered ? b.mask : a.mask;
		}
	}
	for (const auto& [ids, masks] : contacts) {
		QFE::FRAMEWORK::NotifyCollisionTrigger(entityManager, ids.first, ids.second, masks.second, parameterCommands);
		QFE::FRAMEWORK::NotifyCollisionTrigger(entityManager, ids.second, ids.first, masks.first, parameterCommands);
	}
	QFE::FRAMEWORK::EndCollisionTriggerFrame(entityManager, parameterCommands);
	QFE::FRAMEWORK::ApplyCollisionTriggerCommands(entityManager, parameterCommands,
		[&](const std::string& entityPath, uint32_t target) {
			const auto toPath = [](const std::string& value) {
				return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(value.data()), value.size()));
			};
			const auto base = toPath(resources.assetDir);
			const auto relative = toPath(entityPath);
			const auto path = base / relative;
			if (relative.is_absolute() || !QFE::FRAMEWORK::ASSETS::IsPathWithin(path, base)) return;
			QFE::MATH::Vector3 position{};
			const auto* transform = entityManager.GetComponentPtr<QFE::SCENE::TransformComponent>(target);
			const bool hasPosition = transform != nullptr;
			if (hasPosition) position = transform->transform.translate;
			const auto utf8 = path.generic_u8string();
			const uint32_t spawned = QFE::FRAMEWORK::LoadSceneEntity(*sceneManager,
				std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size()));
			if (hasPosition) {
				if (auto* spawnedTransform = entityManager.GetComponentPtr<QFE::SCENE::TransformComponent>(spawned))
					spawnedTransform->transform.translate = position;
			}
		}, [&](const QFE::FRAMEWORK::CollisionTriggerCommand& command) {
			ApplyGameCollisionAction(entityManager, command);
		});
	return true;
}

bool QFE::GAMESYSTEM::PlayerTrackingSystem(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
	QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime) {
	(void)resources;
	(void)deltaTime;

	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);

	// プレイヤー自動トラッキング処理
	std::vector<uint32_t> playerEntityIds;
	std::vector<QFE::MATH::Vector3> playerPositions;
	entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent&) {
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
	(void)resources;

	QFE::SCENE::SceneManager* sceneManager = systems.sceneManager.get();
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);

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
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);

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

					const uint32_t bulletEntityId = QFE::FRAMEWORK::LoadSceneEntity(*sceneManager,
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
		QFE::FRAMEWORK::GetSceneEntities(*systems.sceneManager);
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
				if (!trigger.enabled || !QFE::FRAMEWORK::IsGameActionEnabled(*systems.sceneManager, trigger.inputActionName)) {
					trigger.repeatTimer = 0.0f;
					continue;
				}

				auto getInput = [&](uint32_t triggerMode) {
					bool active = false;
					if (!trigger.inputActionName.empty()) {
						switch (triggerMode) {
						case QFE::STG::BulletEmitterInputTrigger:
							active |= QFE::FRAMEWORK::GetActionTrigger(inputInterface, resources.inputActions, trigger.inputActionName);
							break;
						case QFE::STG::BulletEmitterInputRelease:
							active |= QFE::FRAMEWORK::GetActionRelease(inputInterface, resources.inputActions, trigger.inputActionName);
							break;
						default:
							active |= QFE::FRAMEWORK::GetActionPress(inputInterface, resources.inputActions, trigger.inputActionName);
							break;
						}
					}
					// DirectInput のマウス状態は 4 ボタンまでなので、範囲外を読まない。
					if (trigger.mouseButton >= 0 && trigger.mouseButton <= 3) {
						const int8_t mouseButton = static_cast<int8_t>(trigger.mouseButton);
						switch (triggerMode) {
						case QFE::STG::BulletEmitterInputTrigger:
							active |= QFE::FRAMEWORK::GetMouseTrigger(inputInterface, mouseButton);
							break;
						case QFE::STG::BulletEmitterInputRelease:
							active |= QFE::FRAMEWORK::GetMouseRelease(inputInterface, mouseButton);
							break;
						default:
							active |= QFE::FRAMEWORK::GetMousePress(inputInterface, mouseButton);
							break;
						}
					}
					if (trigger.gamePadButton != 0 && trigger.gamePadButton <= UINT16_MAX) {
						const uint16_t gamePadButton = static_cast<uint16_t>(trigger.gamePadButton);
						switch (triggerMode) {
						case QFE::STG::BulletEmitterInputTrigger:
							active |= QFE::FRAMEWORK::GetGamePadTrigger(inputInterface, gamePadButton);
							break;
						case QFE::STG::BulletEmitterInputRelease:
							active |= QFE::FRAMEWORK::GetGamePadRelease(inputInterface, gamePadButton);
							break;
						default:
							active |= QFE::FRAMEWORK::GetGamePadPress(inputInterface, gamePadButton);
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
