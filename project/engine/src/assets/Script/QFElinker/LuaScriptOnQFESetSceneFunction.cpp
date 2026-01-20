#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSceneFunction.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"

#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"

void QFE::Script::Scene::LuaScriptOnQFESetSceneFunction(sol::state* luaState) {
	luaState->set_function("CreateEntity", [](const std::string& entityName, const Transform& transform) {
		uint32_t id = SceneManager::GetInstance()->RunTimeAddEntity(entityName);
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<Transform>(id)) {
			Transform& t = entityManager->GetComponent<Transform>(id);
			t = transform;
		}
		if (entityManager->HasComponent<ScriptHandles>(id)) {
			ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(id);
			for (const auto& sh : scriptHandles.scriptHandles_) {
				LuaScriptResourceManager::GetInstance()->InitializeScript(sh.handle_);
			}
		}
		return id;
		});

	luaState->set_function("SimpleCreateEntity", [](const std::string& entityName) {
		return SceneManager::GetInstance()->RunTimeAddEntity(entityName);
		});

	luaState->set_function("LoadScene", [](const std::string& sceneName) {
		SceneManager::GetInstance()->RunTimeSwapScene(sceneName);
		});

	luaState->set_function("ChangeModel", [](uint32_t id, const std::string& modelName) {
		SceneManager::GetInstance()->ChangeEntityModel(id, modelName);
		});

	luaState->set_function("ChangeMesh", [](uint32_t id, const std::string& meshName) {
		SceneManager::GetInstance()->ChangeEntityMesh(id, meshName);
		});

	luaState->set_function("DeleteAllTagEntity",[](const std::string& entityTag) {
		EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
		std::vector<uint32_t> entitiesToDelete;
		if (entityManager->HasComponentStrage<SceneObjectData>() == false) {
			return;
		}
		for (const auto& [id, sceneObjectData] : entityManager->GetComponentStrage<SceneObjectData>()) {
			if (sceneObjectData.tag == entityTag) {
				entitiesToDelete.push_back(id);
			}
		}
		for (const auto& id : entitiesToDelete) {
			SceneManager::GetInstance()->DeleteEntity(id);
		}
	});

	luaState->set_function("AddLuaScript", [](uint32_t id, const std::string& scriptName) {
		SceneManager::GetInstance()->RunTimeAddLuaScript(id, scriptName);
		});
}
