#include "engine/include/collider/ColliderManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"

#include "engine/include/core/Math/MyMath.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/scene/Data/SceneObjectData.h"

#ifdef _DEBUG
#include "engine/include/renderer/GraphRenderer.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
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
#ifdef _DEBUG
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponentStrage<SphereColliderData>()) {


		auto& sphereColliderStrage = entityManager->GetComponentStrage<SphereColliderData>();
		for (const auto& pair : sphereColliderStrage) {

			const SphereColliderData& collider = pair.second;
			if (collider.isDraw) {
				GraphRenderer::GetInstance()->DrawCircle(collider.sphere.center, collider.sphere.radius, { 0.0f, 1.0f, 0.0f, 1.0f }, 12);
			}
		}
	}

	if (entityManager->HasComponentStrage<AABBColliderData>()) {

		auto& aabbColliderStrage = entityManager->GetComponentStrage<AABBColliderData>();
		for (const auto& pair : aabbColliderStrage) {
			const AABBColliderData& collider = pair.second;
			if (collider.isDraw) {
				GraphRenderer::GetInstance()->DrawBox(collider.aabb.min, collider.aabb.max, { 0.0f, 1.0f, 0.0f, 1.0f });
			}
		}

	}
#endif // _DEBUG
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

	// 繧ｨ繝ｳ繧ｸ繝ｳ縺悟●豁｢荳ｭ縺ｪ繧牙ｽ薙◆繧雁愛螳壹ｒ陦後ｏ縺ｪ縺・
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
				// SceneObjectData蜿門ｾ・
				if (!entityManager->HasComponent<SceneObjectData>(idA) ||
					!entityManager->HasComponent<SceneObjectData>(idB)) {

					assert(false && "Entities do not have SceneObjectData");
				}

				SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(idA);
				SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(idB);

				// 繧ｿ繧ｰ繝槭せ繧ｯ縺瑚｡晉ｪ∝庄閭ｽ縺・
				if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Tag Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 陦晉ｪ√う繝吶Φ繝医ｒ逋ｺ逕溘＆縺帙ｋ繝ｬ繧､繝､繝ｼ縺・
				if ((colliderA.eventColliderLayer & colliderB.colliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// OnCollisionStay繧､繝吶Φ繝・
				colliderA.isHit = true;
				colliderB.isHit = true;
				luaManager->RunColliderStay(idA, idB, objB);
				luaManager->RunColliderStay(idB, idA, objA);
				// Trigger繧､繝吶Φ繝・
				if (!colliderA.isOldHit) {
					luaManager->RunTriggerEnter(idA, idB, objB);
				}
				if (!colliderB.isOldHit) {
					luaManager->RunTriggerEnter(idB, idA, objA);
				}

				// 蜿咲匱縺励≧繧九Ξ繧､繝､繝ｼ縺・
				if ((colliderA.colliderLayer & colliderB.eventColliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 蜿咲匱蜃ｦ逅・
				// 縺ｩ縺｡繧峨°縺卦rigger縺ｪ繧牙渚逋ｺ縺励↑縺・
				if (colliderA.isTrigger || colliderB.isTrigger) {
					continue;
				}
				// Transform縺後↑縺・↑繧牙渚逋ｺ縺励↑縺・
				if (!entityManager->HasComponent<Transform>(idA) || !entityManager->HasComponent<Transform>(idB)) {
					assert(false && "Entities do not have Transform");
					continue;
				}
				Transform& transformA = entityManager->GetComponent<Transform>(idA);
				Transform& transformB = entityManager->GetComponent<Transform>(idB);

				// 縺ｩ縺｡繧峨ｂ蜍輔￥蝣ｴ蜷医・遲峨＠縺丞渚逋ｺ
				Vector3 length = colliderB.sphere.center - colliderA.sphere.center;
				length -= length.Normalize() * (colliderA.sphere.radius + colliderB.sphere.radius);
				if (colliderA.isStatic == false && colliderB.isStatic == false) {
					transformA.translate += length * 0.5f;
					transformB.translate -= length * 0.5f;
				}
				// 迚・婿縺悟虚縺九↑縺・ｴ蜷医・蜍輔￥譁ｹ縺縺大渚逋ｺ
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
	// 繧ｨ繝ｳ繧ｸ繝ｳ縺悟●豁｢荳ｭ縺ｪ繧牙ｽ薙◆繧雁愛螳壹ｒ陦後ｏ縺ｪ縺・
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
				// SceneObjectData蜿門ｾ・
				if (!entityManager->HasComponent<SceneObjectData>(idA) ||
					!entityManager->HasComponent<SceneObjectData>(idB)) {
					assert(false && "Entities do not have SceneObjectData");
				}
				SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(idA);
				SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(idB);

				// 繧ｿ繧ｰ繝槭せ繧ｯ縺瑚｡晉ｪ∝庄閭ｽ縺・
				if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Tag Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 陦晉ｪ√う繝吶Φ繝医ｒ逋ｺ逕溘＆縺帙ｋ繝ｬ繧､繝､繝ｼ縺・
				if ((colliderA.eventColliderLayer & colliderB.colliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// OnCollisionStay繧､繝吶Φ繝・
				colliderA.isHit = true;
				colliderB.isHit = true;
				luaManager->RunColliderStay(idA, idB, objB);
				luaManager->RunColliderStay(idB, idA, objA);
				// Trigger繧､繝吶Φ繝・
				if (!colliderA.isOldHit) {
					luaManager->RunTriggerEnter(idA, idB, objB);
				}
				if (!colliderB.isOldHit) {
					luaManager->RunTriggerEnter(idB, idA, objA);
				}

				// 蜿咲匱縺励≧繧九Ξ繧､繝､繝ｼ縺・
				if ((colliderA.colliderLayer & colliderB.eventColliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", idA, idB));
#endif // _DEBUG
					continue;
				}

				// 蜿咲匱蜃ｦ逅・
				// 縺ｩ縺｡繧峨°縺卦rigger縺ｪ繧牙渚逋ｺ縺励↑縺・
				if (colliderA.isTrigger || colliderB.isTrigger) {
					continue;
				}
				// Transform縺後↑縺・↑繧牙渚逋ｺ縺励↑縺・
				if (!entityManager->HasComponent<Transform>(idA) || !entityManager->HasComponent<Transform>(idB)) {
					assert(false && "Entities do not have Transform");
					continue;
				}
				Transform& transformA = entityManager->GetComponent<Transform>(idA);
				Transform& transformB = entityManager->GetComponent<Transform>(idB);
				// AABB縺ｮ荳ｭ蠢・ｺｧ讓・
				Vector3 centerA = (colliderA.aabb.min + colliderA.aabb.max) * 0.5f;
				Vector3 centerB = (colliderB.aabb.min + colliderB.aabb.max) * 0.5f;

				// AABB縺ｮ蜊翫し繧､繧ｺ
				Vector3 halfA = (colliderA.aabb.max - colliderA.aabb.min) * 0.51f;
				Vector3 halfB = (colliderB.aabb.max - colliderB.aabb.min) * 0.51f;

				// 荳ｭ蠢・俣霍晞屬
				Vector3 delta = centerB - centerA;
				Vector3 overlap = {
					(halfA.x + halfB.x) - (std::abs(delta.x) + 0.01f),
					(halfA.y + halfB.y) - (std::abs(delta.y) + 0.01f),
					(halfA.z + halfB.z) - (std::abs(delta.z) + 0.01f)
				};

				// 譛蟆上が繝ｼ繝舌・繝ｩ繝・・霆ｸ繧呈爾縺・
				float minOverlap = overlap.x;
				int axis = 0; // 0:x, 1:y, 2:z
				if (overlap.y < minOverlap) { minOverlap = overlap.y; axis = 1; }
				if (overlap.z < minOverlap) { minOverlap = overlap.z; axis = 2; }

				// 蜿咲匱繝吶け繝医Ν繧呈ｱｺ螳・
				Vector3 push(0, 0, 0);
				if (axis == 0) { // x霆ｸ
					push.x = (delta.x > 0) ? minOverlap : -minOverlap;
				} else if (axis == 1) { // y霆ｸ
					push.y = (delta.y > 0) ? minOverlap : -minOverlap;
				} else { // z霆ｸ
					push.z = (delta.z > 0) ? minOverlap : -minOverlap;
				}

				// 縺ｩ縺｡繧峨ｂ蜍輔￥蝣ｴ蜷医・遲峨＠縺丞渚逋ｺ
				if (!colliderA.isStatic && !colliderB.isStatic) {
					transformA.translate -= push * 0.5f;
					transformB.translate += push * 0.5f;
				}
				// 迚・婿縺悟虚縺九↑縺・ｴ蜷医・蜍輔￥譁ｹ縺縺大渚逋ｺ
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
	// 繧ｨ繝ｳ繧ｸ繝ｳ縺悟●豁｢荳ｭ縺ｪ繧牙ｽ薙◆繧雁愛螳壹ｒ陦後ｏ縺ｪ縺・
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
				// SceneObjectData蜿門ｾ・
				if (!entityManager->HasComponent<SceneObjectData>(sphereId) ||
					!entityManager->HasComponent<SceneObjectData>(aabbId)) {
					assert(false && "Entities do not have SceneObjectData");
				}
				SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(sphereId);
				SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(aabbId);

				// 繧ｿ繧ｰ繝槭せ繧ｯ縺瑚｡晉ｪ∝庄閭ｽ縺・
				if (colliderTagMask_.IsCollidable(objA->tag, objB->tag)) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Tag Mismatch between Entity {} and Entity {}", sphereId, aabbId));
#endif // _DEBUG
					continue;
				}

				// 陦晉ｪ√う繝吶Φ繝医ｒ逋ｺ逕溘＆縺帙ｋ繝ｬ繧､繝､繝ｼ縺・
				if ((sphereCollider.eventColliderLayer & aabbCollider.colliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Event Layer Mismatch between Entity {} and Entity {}", sphereId, aabbId));
#endif // _DEBUG
					continue;
				}

				// OnCollisionStay繧､繝吶Φ繝・
				sphereCollider.isHit = true;
				aabbCollider.isHit = true;
				luaManager->RunColliderStay(sphereId, aabbId, objB);
				luaManager->RunColliderStay(aabbId, sphereId, objA);
				// Trigger繧､繝吶Φ繝・
				if (!sphereCollider.isOldHit) {
					luaManager->RunTriggerEnter(sphereId, aabbId, objB);
				}
				if (!aabbCollider.isOldHit) {
					luaManager->RunTriggerEnter(aabbId, sphereId, objA);
				}

				// 蜿咲匱縺励≧繧九Ξ繧､繝､繝ｼ縺・
				if ((sphereCollider.colliderLayer & aabbCollider.eventColliderLayer) == 0) {
#ifdef _DEBUG
					DebugLog(std::format("Collider Repulsion Layer Mismatch between Entity{} and Entity {}", sphereId, aabbId));
#endif // _DEBUG
					continue;
				}

				// 蜿咲匱蜃ｦ逅・
				// 縺ｩ縺｡繧峨°縺卦rigger縺ｪ繧牙渚逋ｺ縺励↑縺・
				if (sphereCollider.isTrigger || aabbCollider.isTrigger) {
					continue;
				}
				// Transform縺後↑縺・↑繧牙渚逋ｺ縺励↑縺・
				if (!entityManager->HasComponent<Transform>(sphereId) || !entityManager->HasComponent<Transform>(aabbId)) {
					assert(false && "Entities do not have Transform");
					continue;
				}
				Transform& transformA = entityManager->GetComponent<Transform>(sphereId);
				Transform& transformB = entityManager->GetComponent<Transform>(aabbId);

				// 譛霑大ｍ轤ｹ繧貞叙蠕・
				Vector3 closestPoint = MyMath::ClosestPoint(sphereCollider.sphere, aabbCollider.aabb);
				Vector3 direction = sphereCollider.sphere.center - closestPoint;
				float distance = direction.Length();

				// 逅・′AABB縺ｫ繧√ｊ霎ｼ繧薙□蛻・□縺第款縺玲綾縺・
				float penetration = sphereCollider.sphere.radius - distance;
				if (penetration > 0.0f && distance > 0.0f) {
					Vector3 push = direction.Normalize() * penetration;
					// 縺ｩ縺｡繧峨ｂ蜍輔￥蝣ｴ蜷医・遲峨＠縺丞渚逋ｺ
					if (!sphereCollider.isStatic && !aabbCollider.isStatic) {
						transformA.translate += push * 0.5f;
						transformB.translate -= push * 0.5f;
					}
					// 迚・婿縺悟虚縺九↑縺・ｴ蜷医・蜍輔￥譁ｹ縺縺大渚逋ｺ
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
