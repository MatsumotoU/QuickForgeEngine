#include "LuaScriptOnQFESetSceneFunction.h"
#include "Assets/AssetManager.h"
#include "Scene/SceneManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"

#include "Core/Math/Transform.h"
#include "Core/Entity/EntityManager.h"
#include "Assets/Script/Data/ScriptHandle.h"

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
