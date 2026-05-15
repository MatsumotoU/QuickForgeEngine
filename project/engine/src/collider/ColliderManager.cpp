#include "engine/include/collider/ColliderManager.h"
#include "engine/include/collider/CollisionDetection.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"

#include "engine/include/core/Math/MyMath.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/Math/TransformComponent.h"
#include "engine/include/scene/Data/SceneObjectData.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/renderer/GraphRenderer.h"
#endif // QFE_OPTIMIZE_OFF

#include "engine/include/core/EngineDefines.h"

#include <algorithm>

namespace QFE {

	void ColliderManager::Initialize() {
		AssetManager* assetManager = AssetManager::GetInstance();
		if (assetManager == nullptr) {
			QFE_REPORT_SYSTEM_ERROR("AssetManager instance is null during ColliderManager initialization.", SystemError::Abort);
		}
		std::string configDir = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Config");
		std::string maskTableFilePath = configDir + "ColliderTagMask.json";

		colliderTagMask_.Initialize(maskTableFilePath);
	}

	void ColliderManager::Update() {
		collisionEnterEntityIds_.clear();
		collisionStayEntityIds_.clear();

		SphereToSphereUpdate();
		AABBToAABBUpdate();
		SphereToAABBUpdate();

		// collisionStayのEntity IDのペアを重複がないようにソートしてユニーク化
		std::sort(collisionStayEntityIds_.begin(), collisionStayEntityIds_.end());
		collisionStayEntityIds_.erase(std
			::unique(collisionStayEntityIds_.begin(), collisionStayEntityIds_.end()), collisionStayEntityIds_.end());

		// collisionEnterのEntity IDのペアを重複がないようにソートしてユニーク化
		std::sort(collisionEnterEntityIds_.begin(), collisionEnterEntityIds_.end());
		collisionEnterEntityIds_.erase(std
			::unique(collisionEnterEntityIds_.begin(), collisionEnterEntityIds_.end()), collisionEnterEntityIds_.end());
	}

	void ColliderManager::Draw() {
#ifdef QFE_OPTIMIZE_OFF
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
		entityManager->Each<SphereColliderData>([&](uint32_t entityId, SphereColliderData& collider) {
			entityId; // 未使用
			if (collider.isDraw) {
				Render::GraphRenderer::GetInstance()->DrawCircle(
					collider.sphere.center, collider.sphere.radius, { 0.0f, 1.0f, 0.0f, 1.0f }, 12);
			}
			});

		entityManager->Each<AABBColliderData>([&](uint32_t entityId, AABBColliderData& collider) {
			entityId; // 未使用
			if (collider.isDraw) {
				Vector3 min = collider.aabb.center - collider.aabb.size * 0.5f;
				Vector3 max = collider.aabb.center + collider.aabb.size * 0.5f;
				Render::GraphRenderer::GetInstance()->DrawBox(min, max, { 0.0f, 1.0f, 0.0f, 1.0f });
			}
			});
#endif // QFE_OPTIMIZE_OFF
	}

	void ColliderManager::Finalize() {
		colliderTagMask_.Finalize();
	}

	void ColliderManager::SphereToSphereUpdate() {
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
		if (!entityManager->HasComponentStrage<SphereColliderData>()) {
			return;
		}

		// SphereColliderDataを持つ全エンティティのIDを取得し、Transformコンポーネントがある場合はSphereの中心を更新する
		std::vector<uint32_t> entityIds;
		entityManager->Each<SphereColliderData>([&](uint32_t entityId, SphereColliderData& collider) {
			entityIds.push_back(entityId);
			if (entityManager->HasComponent<TransformComponent>(entityId)) {
				Transform& transform = entityManager->GetComponent<TransformComponent>(entityId).transform;
				collider.sphere.center = transform.translate;
				collider.isOldHit = collider.isHit;
				collider.isHit = false;
			}
			});

		// エンジンが停止中なら当たり判定を行わない
		if (!isRunning) {
			return;
		}

		for (size_t i = 0; i < entityIds.size(); ++i) {
			uint32_t idA = entityIds[i];
			SphereColliderData& colliderA = entityManager->GetComponent<SphereColliderData>(idA);
			for (size_t j = i + 1; j < entityIds.size(); ++j) {
				uint32_t idB = entityIds[j];
				if (idA == idB) continue;
				SphereColliderData& colliderB = entityManager->GetComponent<SphereColliderData>(idB);
				if (QFE::COLLIDER::isCollision(colliderA.sphere, colliderB.sphere)) {
					// SceneObjectData取得
					if (!entityManager->HasComponent<SceneObjectData>(idA) ||
						!entityManager->HasComponent<SceneObjectData>(idB)) {

						assert(false && "Entities do not have SceneObjectData");
					}

					SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(idA);
					SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(idB);

					// タグマスクが衝突不可か
					if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
						QFE_LOG(std::format("Collider Tag Mismatch between Entity {} and Entity {}", idA, idB));
						continue;
					}

					// 衝突イベントを発生させるレイヤーか
					if ((colliderA.eventColliderLayer & colliderB.colliderLayer) == 0) {
						QFE_LOG(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", idA, idB));
						continue;
					}

					// OnCollisionStayイベント
					colliderA.isHit = true;
					colliderB.isHit = true;
					collisionStayEntityIds_.emplace_back(idA, idB);
					if (!colliderA.isOldHit){
						collisionEnterEntityIds_.emplace_back(idA, idB);
					}

					// 反発しうるレイヤーか
					if ((colliderA.colliderLayer & colliderB.eventColliderLayer) == 0) {
						QFE_LOG(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", idA, idB));
						continue;
					}

					// 反発処理
					// どちらかがTriggerなら反発しない
					if (colliderA.isTrigger || colliderB.isTrigger) {
						continue;
					}
					// Transformがないなら反発しない
					if (!entityManager->HasComponent<TransformComponent>(idA) || !entityManager->HasComponent<TransformComponent>(idB)) {
						assert(false && "Entities do not have Transform");
						continue;
					}
					Transform& transformA = entityManager->GetComponent<TransformComponent>(idA).transform;
					Transform& transformB = entityManager->GetComponent<TransformComponent>(idB).transform;

					// どちらも動く場合は等しく反発
					Vector3 length = colliderB.sphere.center - colliderA.sphere.center;
					length -= length.Normalize() * (colliderA.sphere.radius + colliderB.sphere.radius);
					if (colliderA.isStatic == false && colliderB.isStatic == false) {
						transformA.translate += length * 0.5f;
						transformB.translate -= length * 0.5f;
					}
					// 片方が動かない場合は動く方だけ反発
					else if (colliderA.isStatic == false && colliderB.isStatic == true) {
						transformA.translate += length;
					} else if (colliderA.isStatic == true && colliderB.isStatic == false) {
						transformB.translate -= length;
					}
				}
			}
		}
	}

	void ColliderManager::AABBToAABBUpdate() {
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
		if (!entityManager->HasComponentStrage<AABBColliderData>()) {
			return;
		}

		std::vector<uint32_t> entityIds;
		entityManager->Each<AABBColliderData>([&](uint32_t entityId, AABBColliderData& collider) {
			entityIds.push_back(entityId);
			if (entityManager->HasComponent<TransformComponent>(entityId)) {
				Transform& transform = entityManager->GetComponent<TransformComponent>(entityId).transform;
				collider.aabb.center = transform.translate;
				collider.isOldHit = collider.isHit;
				collider.isHit = false;
			}
			});
		// エンジンが停止中なら当たり判定を行わない
		if (!isRunning) {
			return;
		}
		for (size_t i = 0; i < entityIds.size(); ++i) {
			uint32_t idA = entityIds[i];
			AABBColliderData& colliderA = entityManager->GetComponent<AABBColliderData>(idA);
			for (size_t j = i + 1; j < entityIds.size(); ++j) {
				uint32_t idB = entityIds[j];
				if (idA == idB) continue;
				AABBColliderData& colliderB = entityManager->GetComponent<AABBColliderData>(idB);
				if (QFE::COLLIDER::isCollision(colliderA.aabb, colliderB.aabb)) {
					// SceneObjectData取得
					if (!entityManager->HasComponent<SceneObjectData>(idA) ||
						!entityManager->HasComponent<SceneObjectData>(idB)) {
						assert(false && "Entities do not have SceneObjectData");
					}
					SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(idA);
					SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(idB);

					// タグマスクが衝突不可か
					if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
#ifdef QFE_OPTIMIZE_OFF
						QFE_LOG(std::format("Collider Tag Mismatch between Entity {} and Entity {}", idA, idB));
#endif // QFE_OPTIMIZE_OFF
						continue;
					}

					// 衝突イベントを発生させるレイヤーか
					if ((colliderA.eventColliderLayer & colliderB.colliderLayer) == 0) {
#ifdef QFE_OPTIMIZE_OFF
						QFE_LOG(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", idA, idB));
#endif // QFE_OPTIMIZE_OFF
						continue;
					}

					// OnCollisionStayイベント
					colliderA.isHit = true;
					colliderB.isHit = true;
					collisionStayEntityIds_.emplace_back(idA, idB);
					if (!colliderA.isOldHit) {
						collisionEnterEntityIds_.emplace_back(idA, idB);
					}

					// 反発しうるレイヤーか
					if ((colliderA.colliderLayer & colliderB.eventColliderLayer) == 0) {
#ifdef QFE_OPTIMIZE_OFF
						QFE_LOG(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", idA, idB));
#endif // QFE_OPTIMIZE_OFF
						continue;
					}

					// 反発処理
					// どちらかがTriggerなら反発しない
					if (colliderA.isTrigger || colliderB.isTrigger) {
						continue;
					}
					// Transformがないなら反発しない
					if (!entityManager->HasComponent<TransformComponent>(idA) || !entityManager->HasComponent<TransformComponent>(idB)) {
						assert(false && "Entities do not have Transform");
						continue;
					}
					Transform& transformA = entityManager->GetComponent<TransformComponent>(idA).transform;
					Transform& transformB = entityManager->GetComponent<TransformComponent>(idB).transform;
					// AABBの中心座標
					Vector3 centerA = colliderA.aabb.center;
					Vector3 centerB = colliderB.aabb.center;

					// AABBの半サイズ
					Vector3 halfA = colliderA.aabb.size * 0.5f;
					Vector3 halfB = colliderB.aabb.size * 0.5f;

					// 中心間距離
					Vector3 delta = centerB - centerA;
					Vector3 overlap = {
						(halfA.x + halfB.x) - std::abs(delta.x),
						(halfA.y + halfB.y) - std::abs(delta.y),
						(halfA.z + halfB.z) - std::abs(delta.z)
					};

					// 最小オーバーラップの軸を探す
					float minOverlap = overlap.x;
					int axis = 0; // 0:x, 1:y, 2:z
					if (overlap.y < minOverlap) { minOverlap = overlap.y; axis = 1; }
					if (overlap.z < minOverlap) { minOverlap = overlap.z; axis = 2; }

					// 反発ベクトルを決定
					Vector3 push(0, 0, 0);
					if (axis == 0) { // x軸
						push.x = (delta.x > 0) ? minOverlap : -minOverlap;
					} else if (axis == 1) { // y軸
						push.y = (delta.y > 0) ? minOverlap : -minOverlap;
					} else { // z軸
						push.z = (delta.z > 0) ? minOverlap : -minOverlap;
					}

					// どちらも動く場合は等しく反発
					if (!colliderA.isStatic && !colliderB.isStatic) {
						transformA.translate -= push * 0.5f;
						transformB.translate += push * 0.5f;
					}
					// 片方が動かない場合は動く方だけ反発
					else if (!colliderA.isStatic && colliderB.isStatic) {
						transformA.translate -= push;
					} else if (colliderA.isStatic && !colliderB.isStatic) {
						transformB.translate += push;
					}
				}
			}
		}
	}

	void ColliderManager::SphereToAABBUpdate() {
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
		std::vector<uint32_t> sphereEntityIds;
		entityManager->Each<SphereColliderData>([&](uint32_t entityId, SphereColliderData& collider) {
			if (entityManager->HasComponent<TransformComponent>(entityId)) {
				Transform& transform = entityManager->GetComponent<TransformComponent>(entityId).transform;
				collider.sphere.center = transform.translate;
				collider.isOldHit = collider.isHit;
				collider.isHit = false;
				sphereEntityIds.push_back(entityId);
			}
			});
		std::vector<uint32_t> aabbEntityIds;
		entityManager->Each<AABBColliderData>([&](uint32_t entityId, AABBColliderData& collider) {
			if (entityManager->HasComponent<TransformComponent>(entityId)) {
				Transform& transform = entityManager->GetComponent<TransformComponent>(entityId).transform;
				collider.aabb.center = transform.translate;
				collider.isOldHit = collider.isHit;
				collider.isHit = false;
				aabbEntityIds.push_back(entityId);
			}
			});
	
		// エンジンが停止中なら当たり判定を行わない
		if (!isRunning) {
			return;
		}

		for (int i = 0; i < sphereEntityIds.size(); ++i) {
			uint32_t sphereId = sphereEntityIds[i];
			SphereColliderData& sphereCollider = entityManager->GetComponent<SphereColliderData>(sphereId);
			for (int j = 0; j < aabbEntityIds.size(); ++j) {
				uint32_t aabbId = aabbEntityIds[j];
				if (sphereId == aabbId) continue;
				AABBColliderData& aabbCollider = entityManager->GetComponent<AABBColliderData>(aabbId);
				if (QFE::COLLIDER::isCollision(sphereCollider.sphere, aabbCollider.aabb)) {
					// SceneObjectData取得
					if (!entityManager->HasComponent<SceneObjectData>(sphereId) ||
						!entityManager->HasComponent<SceneObjectData>(aabbId)) {
						assert(false && "Entities do not have SceneObjectData");
					}
					SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(sphereId);
					SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(aabbId);

					// タグマスクが衝突不可か
					if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
						QFE_LOG(std::format("Collider Tag Mismatch between Entity {} and Entity {}", sphereId, aabbId));
						continue;
					}

					// 衝突イベントを発生させるレイヤーか
					if ((sphereCollider.eventColliderLayer & aabbCollider.colliderLayer) == 0) {
						QFE_LOG(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", sphereId, aabbId));
						continue;
					}

					// OnCollisionStayイベント
					sphereCollider.isHit = true;
					aabbCollider.isHit = true;
					collisionStayEntityIds_.emplace_back(sphereId, aabbId);
					if (!sphereCollider.isOldHit) {
						collisionEnterEntityIds_.emplace_back(sphereId, aabbId);
					}

					// 反発しうるレイヤーか
					if ((sphereCollider.colliderLayer & aabbCollider.eventColliderLayer) == 0) {
						QFE_LOG(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", sphereId, aabbId));
						continue;
					}

					// 反発処理
					// どちらかがTriggerなら反発しない
					if (sphereCollider.isTrigger || aabbCollider.isTrigger) {
						continue;
					}
					// Transformがないなら反発しない
					if (!entityManager->HasComponent<TransformComponent>(sphereId) || !entityManager->HasComponent<TransformComponent>(aabbId)) {
						assert(false && "Entities do not have Transform");
						continue;
					}
					Transform& transformA = entityManager->GetComponent<TransformComponent>(sphereId).transform;
					Transform& transformB = entityManager->GetComponent<TransformComponent>(aabbId).transform;

					// 最近点を取得
					Vector3 closestPoint = MyMath::ClosestPoint(sphereCollider.sphere, aabbCollider.aabb);
					Vector3 direction = sphereCollider.sphere.center - closestPoint;
					float distance = direction.Length();

					// 球がAABBにめり込んだ分だけ押し戻す
					float penetration = sphereCollider.sphere.radius - distance;
					if (penetration > 0.0f && distance > 0.0f) {
						Vector3 push = direction.Normalize() * penetration;
						// どちらも動く場合は等しく反発
						if (!sphereCollider.isStatic && !aabbCollider.isStatic) {
							transformA.translate += push * 0.5f;
							transformB.translate -= push * 0.5f;
						}
						// 片方が動かない場合は動く方だけ反発
						else if (!sphereCollider.isStatic && aabbCollider.isStatic) {
							transformA.translate += push;
						} else if (sphereCollider.isStatic && !aabbCollider.isStatic) {
							transformB.translate -= push;
						}
					}
				}
			}
		}
	}
}
