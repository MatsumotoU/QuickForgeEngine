#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetGetterBase.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"

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
	luaState->set_function("SetTranslate", [](uint32_t entityId, Vector3 translate) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<Transform>(entityId)) {
			Transform& t = em->GetComponent<Transform>(entityId);
			t.translate = translate;
		}
		});
	luaState->set_function("SetRotate", [](uint32_t entityId, Vector3 rotate) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<Transform>(entityId)) {
			Transform& t = em->GetComponent<Transform>(entityId);
			t.rotate = rotate;
		}
		});
	luaState->set_function("SetScale", [](uint32_t entityId, Vector3 scale) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<Transform>(entityId)) {
			Transform& t = em->GetComponent<Transform>(entityId);
			t.scale = scale;
		}
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

	luaState->set_function("GetEntityTag", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<SceneObjectData>(entityId)) {
			return em->GetComponent<SceneObjectData>(entityId).tag;
		}
		return std::string{};
		});

	luaState->set_function("SetEntityTag", [](uint32_t entityId, const std::string& tag) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<SceneObjectData>(entityId)) {
			em->GetComponent<SceneObjectData>(entityId).tag = tag;
		}
		});

	luaState->set_function("GetEntityName", [](uint32_t entityId) {
		auto* em = AssetManager::GetInstance()->GetEntityManager();
		if (em->HasComponent<SceneObjectData>(entityId)) {
			return em->GetComponent<SceneObjectData>(entityId).name;
		}
		return std::string{};
		});

	luaState->set_function("SetScore", [](int32_t score) {
		SceneManager::GetInstance()->SetScore(score);
		});
	luaState->set_function("GetScore", []() {
		return SceneManager::GetInstance()->GetScore();
		});

	luaState->set_function("SetSceneGlobalData", [](const std::string& key, const sol::object& value) {
		auto& globalData = SceneManager::GetInstance()->GetSceneGlobalData();
		if (value.is<int>()) {
			globalData[key] = value.as<int>();
		}
		else if (value.is<double>()) {
			globalData[key] = value.as<double>();
		}
		else if (value.is<std::string>()) {
			globalData[key] = value.as<std::string>();
		}
		else if (value.is<bool>()) {
			globalData[key] = value.as<bool>();
		}
		else {
			globalData[key] = nullptr;
		}
		});

	luaState->set_function("GetSceneGlobalData", [](const std::string& key, sol::this_state ts) {
		auto& globalData = SceneManager::GetInstance()->GetSceneGlobalData();

		if (globalData.contains(key)) {
			if (globalData[key].is_number_integer()) {
				return sol::object(sol::make_object(ts, globalData[key].get<int>()));
			}
			else if (globalData[key].is_string()) {
				return sol::object(sol::make_object(ts, globalData[key].get<std::string>()));
			}
			else if (globalData[key].is_boolean()) {
				return sol::object(sol::make_object(ts, globalData[key].get<bool>()));
			}
			else if (globalData[key].is_number_float()) {
				return sol::object(sol::make_object(ts, globalData[key].get<double>()));
			}
		}
		return sol::object(sol::nil);
		});
}
