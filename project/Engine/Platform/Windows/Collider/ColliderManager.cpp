#include "ColliderManager.h"
#include "Assets/AssetManager.h"
#include "Core/Entity/EntityManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"

#include "Core/Math/MyMath.h"
#include "Core/Math/Transform.h"
#include "Scene/Data/SceneObjectData.h"

#ifdef _DEBUG
#include "Renderer/GraphRenderer.h"
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include <algorithm>

void ColliderManager::Initialize() {
}

void ColliderManager::Update() {
	SphereToSphereUpdate();
	AABBToAABBUpdate();
	SphereToAABBUpdate();
}

void ColliderManager::Draw() {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponentStrage<SphereColliderData>()) {
		auto& sphereColliderStrage = entityManager->GetComponentStrage<SphereColliderData>();
#ifdef _DEBUG
		for (const auto& pair : sphereColliderStrage) {

			const SphereColliderData& collider = pair.second;
			if (collider.isDraw) {
				GraphRenderer::GetInstance()->DrawCircle(collider.sphere.center, collider.sphere.radius, { 0.0f, 1.0f, 0.0f, 1.0f }, 12);
			}
		}
#endif // _DEBUG
	}

	if (entityManager->HasComponentStrage<AABBColliderData>()) {
		auto& aabbColliderStrage = entityManager->GetComponentStrage<AABBColliderData>();
#ifdef _DEBUG
		for (const auto& pair : aabbColliderStrage) {
			const AABBColliderData& collider = pair.second;
			if (collider.isDraw) {
				GraphRenderer::GetInstance()->DrawBox(collider.aabb.min, collider.aabb.max, { 0.0f, 1.0f, 0.0f, 1.0f });
			}
		}
#endif // _DEBUG
	}
}

void ColliderManager::Finalize() {
}

bool ColliderManager::isCollision(const Sphere& sphere1, const Sphere& sphere2) {
	if ((sphere1.center - sphere2.center).Length() <= sphere1.radius + sphere2.radius) {
		return true;
	} else {
		return false;
	}
}

bool ColliderManager::isCollision(const AABB& aabb1, const AABB& aabb2) {
	if (aabb1.max.x < aabb2.min.x || aabb1.min.x > aabb2.max.x) { return false; }
	if (aabb1.max.y < aabb2.min.y || aabb1.min.y > aabb2.max.y) { return false; }
	if (aabb1.max.z < aabb2.min.z || aabb1.min.z > aabb2.max.z) { return false; }
	return true;
}

bool ColliderManager::isCollision(const Sphere& sphere, const AABB& aabb) {
	Vector3 closestPoint = MyMath::ClosestPoint(sphere, aabb);
	float distanceSquared = (closestPoint - sphere.center).LengthSq();
	return distanceSquared <= (sphere.radius * sphere.radius);
}

void ColliderManager::SphereToSphereUpdate() {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponentStrage<SphereColliderData>()) {
		return;
	}

	auto& sphereColliderStrage = entityManager->GetComponentStrage<SphereColliderData>();
	std::vector<uint32_t> entityIds;
	for (auto& pair : sphereColliderStrage) {
		entityIds.push_back(pair.first);
		if (entityManager->HasComponent<Transform>(pair.first)) {
			Transform& transform = entityManager->GetComponent<Transform>(pair.first);
			SphereColliderData& collider = entityManager->GetComponent<SphereColliderData>(pair.first);
			collider.sphere.center = transform.translate;
			collider.isOldHit = collider.isHit;
			collider.isHit = false;
		}
	}

	// エンジンが停止中なら当たり判定を行わない
	if (!isRunning) {
		return;
	}

	for (size_t i = 0; i < entityIds.size(); ++i) {
		uint32_t idA = entityIds[i];
		SphereColliderData& colliderA = entityManager->GetComponent<SphereColliderData>(idA);
		for (size_t j = i + 1; j < entityIds.size(); ++j) {
			uint32_t idB = entityIds[j];
			SphereColliderData& colliderB = entityManager->GetComponent<SphereColliderData>(idB);
			if (isCollision(colliderA.sphere, colliderB.sphere)) {
				LuaScriptResourceManager* luaManager = LuaScriptResourceManager::GetInstance();
				// SceneObjectData取得
				if (!entityManager->HasComponent<SceneObjectData>(idA) ||
					!entityManager->HasComponent<SceneObjectData>(idB)) {

					assert(false && "Entities do not have SceneObjectData");
				}

				SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(idA);
				SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(idB);

				// タグマスクが衝突可能か
				if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Tag Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 衝突イベントを発生させるレイヤーか
				if ((colliderA.eventColliderLayer & colliderB.colliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// OnCollisionStayイベント
				colliderA.isHit = true;
				colliderB.isHit = true;
				luaManager->RunColliderStay(idA, idB, objB);
				luaManager->RunColliderStay(idB, idA, objA);
				// Triggerイベント
				if (!colliderA.isOldHit) {
					luaManager->RunTriggerEnter(idA, idB, objB);
				}
				if (!colliderB.isOldHit) {
					luaManager->RunTriggerEnter(idB, idA, objA);
				}

				// 反発しうるレイヤーか
				if ((colliderA.colliderLayer & colliderB.eventColliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 反発処理
				// どちらかがTriggerなら反発しない
				if (colliderA.isTrigger || colliderB.isTrigger) {
					continue;
				}
				// Transformがないなら反発しない
				if (!entityManager->HasComponent<Transform>(idA) || !entityManager->HasComponent<Transform>(idB)) {
					assert(false && "Entities do not have Transform");
					continue;
				}
				Transform& transformA = entityManager->GetComponent<Transform>(idA);
				Transform& transformB = entityManager->GetComponent<Transform>(idB);

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
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponentStrage<AABBColliderData>()) {
		return;
	}
	auto& aabbColliderStrage = entityManager->GetComponentStrage<AABBColliderData>();
	std::vector<uint32_t> entityIds;
	for (auto& pair : aabbColliderStrage) {
		entityIds.push_back(pair.first);
		if (entityManager->HasComponent<Transform>(pair.first)) {
			Transform& transform = entityManager->GetComponent<Transform>(pair.first);
			AABBColliderData& collider = entityManager->GetComponent<AABBColliderData>(pair.first);
			Vector3 halfSize = (collider.aabb.max - collider.aabb.min) * 0.5f;
			collider.aabb.min = transform.translate - halfSize;
			collider.aabb.max = transform.translate + halfSize;
			collider.isOldHit = collider.isHit;
			collider.isHit = false;
		}
	}
	// エンジンが停止中なら当たり判定を行わない
	if (!isRunning) {
		return;
	}
	for (size_t i = 0; i < entityIds.size(); ++i) {
		uint32_t idA = entityIds[i];
		AABBColliderData& colliderA = entityManager->GetComponent<AABBColliderData>(idA);
		for (size_t j = i + 1; j < entityIds.size(); ++j) {
			uint32_t idB = entityIds[j];
			AABBColliderData& colliderB = entityManager->GetComponent<AABBColliderData>(idB);
			if (isCollision(colliderA.aabb, colliderB.aabb)) {
				LuaScriptResourceManager* luaManager = LuaScriptResourceManager::GetInstance();
				// SceneObjectData取得
				if (!entityManager->HasComponent<SceneObjectData>(idA) ||
					!entityManager->HasComponent<SceneObjectData>(idB)) {
					assert(false && "Entities do not have SceneObjectData");
				}
				SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(idA);
				SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(idB);

				// タグマスクが衝突可能か
				if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Tag Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 衝突イベントを発生させるレイヤーか
				if ((colliderA.eventColliderLayer & colliderB.colliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// OnCollisionStayイベント
				colliderA.isHit = true;
				colliderB.isHit = true;
				luaManager->RunColliderStay(idA, idB, objB);
				luaManager->RunColliderStay(idB, idA, objA);
				// Triggerイベント
				if (!colliderA.isOldHit) {
					luaManager->RunTriggerEnter(idA, idB, objB);
				}
				if (!colliderB.isOldHit) {
					luaManager->RunTriggerEnter(idB, idA, objA);
				}

				// 反発しうるレイヤーか
				if ((colliderA.colliderLayer & colliderB.eventColliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 反発処理
				// どちらかがTriggerなら反発しない
				if (colliderA.isTrigger || colliderB.isTrigger) {
					continue;
				}
				// Transformがないなら反発しない
				if (!entityManager->HasComponent<Transform>(idA) || !entityManager->HasComponent<Transform>(idB)) {
					assert(false && "Entities do not have Transform");
					continue;
				}
				Transform& transformA = entityManager->GetComponent<Transform>(idA);
				Transform& transformB = entityManager->GetComponent<Transform>(idB);
				// AABBの中心座標
				Vector3 centerA = (colliderA.aabb.min + colliderA.aabb.max) * 0.5f;
				Vector3 centerB = (colliderB.aabb.min + colliderB.aabb.max) * 0.5f;

				// AABBの半サイズ
				Vector3 halfA = (colliderA.aabb.max - colliderA.aabb.min) * 0.51f;
				Vector3 halfB = (colliderB.aabb.max - colliderB.aabb.min) * 0.51f;

				// 中心間距離
				Vector3 delta = centerB - centerA;
				Vector3 overlap = {
					(halfA.x + halfB.x) - (std::abs(delta.x) + 0.01f),
					(halfA.y + halfB.y) - (std::abs(delta.y) + 0.01f),
					(halfA.z + halfB.z) - (std::abs(delta.z) + 0.01f)
				};

				// 最小オーバーラップ軸を探す
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
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponentStrage<SphereColliderData>()) {
		return;
	}
	if (!entityManager->HasComponentStrage<AABBColliderData>()) {
		return;
	}
	auto& sphereColliderStrage = entityManager->GetComponentStrage<SphereColliderData>();
	auto& aabbColliderStrage = entityManager->GetComponentStrage<AABBColliderData>();
	std::vector<uint32_t> sphereEntityIds;
	for (auto& pair : sphereColliderStrage) {
		sphereEntityIds.push_back(pair.first);
		if (entityManager->HasComponent<Transform>(pair.first)) {
			Transform& transform = entityManager->GetComponent<Transform>(pair.first);
			SphereColliderData& collider = entityManager->GetComponent<SphereColliderData>(pair.first);
			collider.sphere.center = transform.translate;
			collider.isOldHit = collider.isHit;
			collider.isHit = false;
		}
	}
	std::vector<uint32_t> aabbEntityIds;
	for (auto& pair : aabbColliderStrage) {
		aabbEntityIds.push_back(pair.first);
		if (entityManager->HasComponent<Transform>(pair.first)) {
			Transform& transform = entityManager->GetComponent<Transform>(pair.first);
			AABBColliderData& collider = entityManager->GetComponent<AABBColliderData>(pair.first);
			Vector3 halfSize = (collider.aabb.max - collider.aabb.min) * 0.5f;
			collider.aabb.min = transform.translate - halfSize;
			collider.aabb.max = transform.translate + halfSize;
			collider.isOldHit = collider.isHit;
			collider.isHit = false;
		}
	}
	// エンジンが停止中なら当たり判定を行わない
	if (!isRunning) {
		return;
	}

	for (int i = 0; i < sphereEntityIds.size(); ++i) {
		uint32_t sphereId = sphereEntityIds[i];
		SphereColliderData& sphereCollider = entityManager->GetComponent<SphereColliderData>(sphereId);
		for (int j = 0; j < aabbEntityIds.size(); ++j) {
			uint32_t aabbId = aabbEntityIds[j];
			AABBColliderData& aabbCollider = entityManager->GetComponent<AABBColliderData>(aabbId);
			if (isCollision(sphereCollider.sphere, aabbCollider.aabb)) {
				LuaScriptResourceManager* luaManager = LuaScriptResourceManager::GetInstance();
				// SceneObjectData取得
				if (!entityManager->HasComponent<SceneObjectData>(sphereId) ||
					!entityManager->HasComponent<SceneObjectData>(aabbId)) {
					assert(false && "Entities do not have SceneObjectData");
				}
				SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(sphereId);
				SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(aabbId);

				// タグマスクが衝突可能か
				if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Tag Mismatch between Entity {} and Entity {}", sphereId, aabbId));
#endif // _DEBUG
					continue;
				}

				// 衝突イベントを発生させるレイヤーか
				if ((sphereCollider.eventColliderLayer & aabbCollider.colliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", sphereId, aabbId));
#endif // _DEBUG
					continue;
				}

				// OnCollisionStayイベント
				sphereCollider.isHit = true;
				aabbCollider.isHit = true;
				luaManager->RunColliderStay(sphereId, aabbId, objB);
				luaManager->RunColliderStay(aabbId, sphereId, objA);
				// Triggerイベント
				if (!sphereCollider.isOldHit) {
					luaManager->RunTriggerEnter(sphereId, aabbId, objB);
				}
				if (!aabbCollider.isOldHit) {
					luaManager->RunTriggerEnter(aabbId, sphereId, objA);
				}

				// 反発しうるレイヤーか
				if ((sphereCollider.colliderLayer & aabbCollider.eventColliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", sphereId, aabbId));
#endif // _DEBUG
					continue;
				}

				// 反発処理
				// どちらかがTriggerなら反発しない
				if (sphereCollider.isTrigger || aabbCollider.isTrigger) {
					continue;
				}
				// Transformがないなら反発しない
				if (!entityManager->HasComponent<Transform>(sphereId) || !entityManager->HasComponent<Transform>(aabbId)) {
					assert(false && "Entities do not have Transform");
					continue;
				}
				Transform& transformA = entityManager->GetComponent<Transform>(sphereId);
				Transform& transformB = entityManager->GetComponent<Transform>(aabbId);

				// 最近傍点を取得
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
