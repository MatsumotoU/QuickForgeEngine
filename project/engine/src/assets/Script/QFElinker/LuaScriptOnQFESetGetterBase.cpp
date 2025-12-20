#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetGetterBase.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"

#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/assets/Sprite/Data/SpriteData.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"

#include "engine/include/collider/Data/AABBColliderData.h"
#include "engine/include/collider/Data/SphereColliderData.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/physics/Force.h"
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#include "engine/include/core/EngineGlobalValue.h"

void QFE::Script::Base::LuaScriptOnQFESetGetterBase(sol::state* luaState) {
	luaState->set_function("GetDeltaTime", []() {return QFE::EngineGlobalValue::deltaTime; });

	luaState->set_function("GetEntity", [](const std::string& entityName) {
		return SceneManager::GetInstance()->GetEntityByName(entityName);
		});
	luaState->set_function("GetEntityFromUniqeID", [](uint32_t uniqeId) {
		return SceneManager::GetInstance()->GetEntityByUniqeID(uniqeId);
		}
	);

	luaState->set_function("GetIsDraw", [](uint32_t entityId) {
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<SpriteData>(entityId)) {
			SpriteData& sprite = entityManager->GetComponent<SpriteData>(entityId);
			return sprite.isDraw;
		}
		return false;
		});
	luaState->set_function("SetIsDraw", [](uint32_t entityId, bool isDraw) {
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<SpriteData>(entityId)) {
			SpriteData& sprite = entityManager->GetComponent<SpriteData>(entityId);
			sprite.isDraw = isDraw;
		}
		});

	luaState->set_function("GetTransform", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		return em->HasComponent<Transform>(entityId) ? &em->GetComponent<Transform>(entityId) : nullptr;
		});
	luaState->set_function("GetSceneObjectData", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		return em->HasComponent<SceneObjectData>(entityId) ? &em->GetComponent<SceneObjectData>(entityId) : nullptr;
		});
	luaState->set_function("GetForce", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		return em->HasComponent<Force>(entityId) ? &em->GetComponent<Force>(entityId) : nullptr;
		});
	luaState->set_function("GetMaterial", [](uint32_t entityId) -> Material* {
		auto* am = AssetManager::GetInstance();
		auto* em = am->GetEntityManager();
		auto* gpuBufferPool = am->GetGpuBufferPool();
		if (em->HasComponent<SpriteData>(entityId)) {
			return gpuBufferPool->GetConstantBuffer<Material>(
				em->GetComponent<SpriteData>(entityId).materialBufferHandle
			)->GetData();
		}
		if (em->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& modelData = em->GetComponent<ModelHandle>(entityId);
			ModelRenderData* modelRenderData = am->GetModelRenderData(modelData.handle);
			return gpuBufferPool->GetConstantBuffer<Material>(
				modelRenderData->meshRenderDataHandles[0].materialHandle
			)->GetData();
		}
#ifdef DEBUG
		DebugLog("GetMaterial: Entity does not have SpriteData or ModelHandle.", LogLevel::Warning);
#endif // DEBUG
		return nullptr;
		});

	luaState->set_function("GetColliderIsTrigger", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<SphereColliderData>(entityId)) {
			return em->GetComponent<SphereColliderData>(entityId).isTrigger;
		}
		if (em->HasComponent<AABBColliderData>(entityId)) {
			return em->GetComponent<AABBColliderData>(entityId).isTrigger;
		}
		return false;
		});
	luaState->set_function("SetColliderIsTrigger", [](uint32_t entityId, bool isTrigger) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<SphereColliderData>(entityId)) {
			em->GetComponent<SphereColliderData>(entityId).isTrigger = isTrigger;
		}
		if (em->HasComponent<AABBColliderData>(entityId)) {
			em->GetComponent<AABBColliderData>(entityId).isTrigger = isTrigger;
		}
		});
}
