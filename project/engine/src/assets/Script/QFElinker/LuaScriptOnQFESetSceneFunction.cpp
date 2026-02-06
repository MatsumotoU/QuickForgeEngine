#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSceneFunction.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"

#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/assets/3DModel/Loader/AssimpModelLoader.h"
#include "engine/include/renderer/ModelRenderer.h"

#include "engine/include/assets/Script/LuaScriptExecutor.h"

void QFE::Script::Scene::LuaScriptOnQFESetSceneFunction(sol::state* luaState, EntityManager* entityManager, LuaScriptExecutor* luaScriptExecutor) {
	luaState->set_function("CreateEntity", [entityManager, luaScriptExecutor](const std::string& entityName, const Transform& transform) {
		uint32_t id = SceneManager::GetInstance()->RunTimeAddEntity(entityName);
		
		if (entityManager->HasComponent<Transform>(id)) {
			Transform& t = entityManager->GetComponent<Transform>(id);
			t = transform;
		}
		if (entityManager->HasComponent<ScriptHandles>(id)) {
			ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(id);
			for (const auto& sh : scriptHandles.scriptHandles_) {
				auto* script = luaScriptExecutor->GetScript(sh.handle_);
				if (script) {
					script->RunFunction("Init");
				}
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

	luaState->set_function("ChangeModel", [entityManager](uint32_t id, const std::string& modelName) {
		if (!entityManager->HasComponent<ModelHandle>(id)) { return; }
		ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(id);
		modelHandle.modelName = modelName;
		modelHandle.handle = AssetManager::GetInstance()->LoadModel(modelName);
		});

	luaState->set_function("ChangeMesh", [entityManager](uint32_t id, const std::string& meshName) {
		AssetManager* assetManager = AssetManager::GetInstance();
		if (!entityManager->HasComponent<ModelHandle>(id)) { return; }
		ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(id);
		ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
		if (modelData->meshRenderDataHandles.size() == 0) { return; }
		modelData->meshRenderDataHandles[0].vertexBufferHandle = assetManager->LoadModelMesh(meshName);
		});

	luaState->set_function("DeleteAllTagEntity",[entityManager](const std::string& entityTag) {
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
			entityManager->RemoveEntity(id);
		}
	});

	luaState->set_function("AddLuaScript", [](uint32_t id, const std::string& scriptName) {
		SceneManager::GetInstance()->RunTimeAddLuaScript(id, scriptName);
		});
}
