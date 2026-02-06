#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetGetterBase.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"

#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/assets/Sprite/Data/SpriteData.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"

#include "engine/include/collider/Data/AABBColliderData.h"
#include "engine/include/collider/Data/SphereColliderData.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/physics/Force.h"
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#include "engine/include/core/EngineGlobalValue.h"

void QFE::Script::Base::LuaScriptOnQFESetGetterBase(sol::state* luaState, EntityManager* entityManager) {
	luaState->set_function("GetDeltaTime", []() {return QFE::EngineGlobalValue::deltaTime; });

	luaState->set_function("GetEntity", [entityManager](const std::string& entityName) {
		if (entityManager->HasComponentStrage<SceneObjectData>()) {
			for (const auto& [id, sceneObjectData] : entityManager->GetComponentStrage<SceneObjectData>()) {
				if (sceneObjectData.name == entityName) {
					return id;
				}
			}
		}
		return (uint32_t)0;
		});
	luaState->set_function("GetEntityFromUniqeID", [entityManager](uint32_t uniqeId) {
		if (entityManager->HasComponentStrage<SceneObjectData>()) {
			for (const auto& [id, sceneObjectData] : entityManager->GetComponentStrage<SceneObjectData>()) {
				if (sceneObjectData.uniqueId == uniqeId) {
					return id;
				}
			}
		}
		return (uint32_t)0;
		}
	);

	luaState->set_function("GetIsDraw", [entityManager](uint32_t entityId) {
		if (entityManager->HasComponent<SpriteData>(entityId)) {
			SpriteData& sprite = entityManager->GetComponent<SpriteData>(entityId);
			return sprite.isDraw;
		}
		return false;
		});
	luaState->set_function("SetIsDraw", [entityManager](uint32_t entityId, bool isDraw) {
		if (entityManager->HasComponent<SpriteData>(entityId)) {
			SpriteData& sprite = entityManager->GetComponent<SpriteData>(entityId);
			sprite.isDraw = isDraw;
		}
		});

	luaState->set_function("GetTransform", [entityManager](uint32_t entityId) {
		return entityManager->HasComponent<Transform>(entityId) ? &entityManager->GetComponent<Transform>(entityId) : nullptr;
		});
	luaState->set_function("SetTranslate", [entityManager](uint32_t entityId, Vector3 translate) {
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& t = entityManager->GetComponent<Transform>(entityId);
			t.translate = translate;
		}
		});
	luaState->set_function("SetRotate", [entityManager](uint32_t entityId, Vector3 rotate) {
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& t = entityManager->GetComponent<Transform>(entityId);
			t.rotate = rotate;
		}
		});
	luaState->set_function("SetScale", [entityManager](uint32_t entityId, Vector3 scale) {
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& t = entityManager->GetComponent<Transform>(entityId);
			t.scale = scale;
		}
		});
	luaState->set_function("GetSceneObjectData", [entityManager](uint32_t entityId) {
		return entityManager->HasComponent<SceneObjectData>(entityId) ? &entityManager->GetComponent<SceneObjectData>(entityId) : nullptr;
		});
	luaState->set_function("GetForce", [entityManager](uint32_t entityId) {
		return entityManager->HasComponent<Force>(entityId) ? &entityManager->GetComponent<Force>(entityId) : nullptr;
		});
	luaState->set_function("GetMaterial", [entityManager](uint32_t entityId) -> Material* {
		auto* am = AssetManager::GetInstance();
		auto* gpuBufferPool = am->GetGpuBufferPool();
		if (entityManager->HasComponent<SpriteData>(entityId)) {
			return gpuBufferPool->GetConstantBuffer<Material>(
				entityManager->GetComponent<SpriteData>(entityId).materialBufferHandle
			)->GetData();
		}
		if (entityManager->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& modelData = entityManager->GetComponent<ModelHandle>(entityId);
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

	luaState->set_function("GetColliderIsTrigger", [entityManager](uint32_t entityId) {
		if (entityManager->HasComponent<SphereColliderData>(entityId)) {
			return entityManager->GetComponent<SphereColliderData>(entityId).isTrigger;
		}
		if (entityManager->HasComponent<AABBColliderData>(entityId)) {
			return entityManager->GetComponent<AABBColliderData>(entityId).isTrigger;
		}
		return false;
		});
	luaState->set_function("SetColliderIsTrigger", [entityManager](uint32_t entityId, bool isTrigger) {
		if (entityManager->HasComponent<SphereColliderData>(entityId)) {
			entityManager->GetComponent<SphereColliderData>(entityId).isTrigger = isTrigger;
		}
		if (entityManager->HasComponent<AABBColliderData>(entityId)) {
			entityManager->GetComponent<AABBColliderData>(entityId).isTrigger = isTrigger;
		}
		});

	luaState->set_function("GetEntityTag", [entityManager](uint32_t entityId) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			return entityManager->GetComponent<SceneObjectData>(entityId).tag;
		}
		return std::string{};
		});

	luaState->set_function("SetEntityTag", [entityManager](uint32_t entityId, const std::string& tag) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			entityManager->GetComponent<SceneObjectData>(entityId).tag = tag;
		}
		});

	luaState->set_function("GetEntityName", [entityManager](uint32_t entityId) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			return entityManager->GetComponent<SceneObjectData>(entityId).name;
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
