#include "LuaScriptOnQFESetSceneFunction.h"
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

	luaState->set_function("LoadScene", [](const std::string& sceneName) {
		SceneManager::GetInstance()->RunTimeSwapScene(sceneName);
		});
}
