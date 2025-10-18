#include "LuaScriptOnQFESetSceneFunction.h"
#include "Assets/AssetManager.h"
#include "Scene/SceneManager.h"

#include "Core/Math/Transform.h"

void QFE::Script::Scene::LuaScriptOnQFESetSceneFunction(sol::state* luaState) {
	luaState->set_function("CreateEntity", [](const std::string& entityName, const Transform& transform) {
		uint32_t id = SceneManager::GetInstance()->RunTimeAddEntity(entityName);
		AssetManager* assetManager = AssetManager::GetInstance();
		EntityManager* entityManager = assetManager->GetEntityManager();
		if (entityManager->HasComponent<Transform>(id)) {
			Transform& t = entityManager->GetComponent<Transform>(id);
			t = transform;
		}
		return id;
		});

	luaState->set_function("LoadScene", [](const std::string& sceneName) {
		SceneManager::GetInstance()->RunTimeSwapScene(sceneName);
		});
}
