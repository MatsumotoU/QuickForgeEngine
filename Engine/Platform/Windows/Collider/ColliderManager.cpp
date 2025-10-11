#include "ColliderManager.h"
#include "Assets/AssetManager.h"
#include "Core/Entity/EntityManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"

#include "Core/Math/Transform.h"
#include "Scene/Data/SceneObjectData.h"

#ifdef _DEBUG
#include "Renderer/GraphRenderer.h"
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void ColliderManager::Initialize() {
}

void ColliderManager::Update() {
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
#ifdef _DEBUG
				LuaScriptResourceManager* luaManager = LuaScriptResourceManager::GetInstance();
				// SceneObjectData取得
				if (!entityManager->HasComponent<SceneObjectData>(idA) ||
					!entityManager->HasComponent<SceneObjectData>(idB)) {

					assert(false && "Entities do not have SceneObjectData");
				}

				SceneObjectData* objA = &entityManager->GetComponent<SceneObjectData>(idA);
				SceneObjectData* objB = &entityManager->GetComponent<SceneObjectData>(idB);

				// OnCollisionStayイベント
				colliderA.isHit = true;
				colliderB.isHit = true;
				luaManager->RunColliderStay(idA, idB, objB);
				luaManager->RunColliderStay(idB,idA, objA);

				// Triggerイベント
				if (!colliderA.isOldHit) {
					luaManager->RunTriggerEnter(idA, idB, objB);
				}
				if (!colliderB.isOldHit) {
					luaManager->RunTriggerEnter(idB, idA, objA);
				}
#endif // _DEBUG
			}
		}
	}
}

void ColliderManager::Draw() {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponentStrage<SphereColliderData>()) {
		return;
	}

	auto& sphereColliderStrage = entityManager->GetComponentStrage<SphereColliderData>();
	for (const auto& pair : sphereColliderStrage) {
#ifdef _DEBUG
		const SphereColliderData& collider = pair.second;
		if (collider.isDraw) {
			GraphRenderer::GetInstance()->DrawCircle(collider.sphere.center, collider.sphere.radius, { 0.0f, 1.0f, 0.0f, 1.0f }, 12);
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