#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetUtilities.h"
#include "engine/include/assets/AssetManager.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include "engine/include/utility/FileSystems/FileUtility.h"
#include "Engine/include/scene/Data/SceneObjectData.h"
#include "Engine/include/assets/Script/LuaScriptResourceManager.h"
#include "Engine/include/core/Math/Transform.h"
#include <algorithm>

void QFE::Script::Utility::LuaScriptOnQFESetUtility(sol::state* luaState) {


	// CSV読み込み
	luaState->set_function("Load2DMap", [](const std::string& fileName) {
		std::vector<std::vector<uint32_t>> result;
		std::string path = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("2DMap") + fileName;
#ifdef _DEBUG
		DebugLog("Load2DMap: " + path);
#endif // _DEBUG

		if (QFE::FILE::LoadCSVToVector(path, result)) {
			return sol::as_table(result);
		}
		else {
#ifdef _DEBUG
			DebugLog("Failed to load 2D map: " + path, LogLevel::Error);
#endif // _DEBUG
			return sol::as_table(std::vector<std::vector<uint32_t>>{});
		}
		});

	luaState->set_function("CountEntityName", [](const std::string& entityName) {
		int32_t count = 0;
		EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
		if (entityManager->HasComponentStrage<SceneObjectData>() == false) {
			return 0;
		}
		for (const auto& [id, sceneObjectData] : entityManager->GetComponentStrage<SceneObjectData>()) {
			if (sceneObjectData.name == entityName) {
				++count;
			}
		}
		return count;
		}
	);

	luaState->set_function("CountEntityTag", [](const std::string& entityTag) {
		int32_t count = 0;
		EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
		if (entityManager->HasComponentStrage<SceneObjectData>() == false) {
			return 0;
		}
		for (const auto& [id, sceneObjectData] : entityManager->GetComponentStrage<SceneObjectData>()) {
			if (sceneObjectData.tag == entityTag) {
				++count;
			}
		}
		return count;
		}
	);

	luaState->set_function("GetEntitiesByTag", [](const std::string& entityTag, sol::this_state ts) {
		sol::state_view lua(ts);
		sol::table result = lua.create_table();
		EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
		if (entityManager->HasComponentStrage<SceneObjectData>() == false) {
			return result;
		}

		std::vector<uint32_t> entityIds;
		for (const auto& [id, sceneObjectData] : entityManager->GetComponentStrage<SceneObjectData>()) {
			if (sceneObjectData.tag == entityTag && entityManager->IsActiveEntity(id) && entityManager->HasComponent<Transform>(id)) {
				entityIds.push_back(id);
			}
		}
		std::sort(entityIds.begin(), entityIds.end());
		for (size_t index = 0; index < entityIds.size(); ++index) {
			result[index + 1] = entityIds[index];
		}
		return result;
		});

	luaState->set_function("RunAllFunction", [](const std::string& message) {
		LuaScriptResourceManager::GetInstance()->RunAllFunction(message);
		});

	luaState->set_function("GetMinLengthToEntityFromTag", [](const std::string& entityTag, const Vector3& fromPosition) {
		float minLength = FLT_MAX;
		EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
		if (entityManager->HasComponentStrage<SceneObjectData>() == false) {
			return minLength;
		}
		for (const auto& [id, sceneObjectData] : entityManager->GetComponentStrage<SceneObjectData>()) {
			if (sceneObjectData.tag == entityTag) {
				if (entityManager->HasComponent<Transform>(id)) {
					const Transform& transform = entityManager->GetComponent<Transform>(id);
					float length = (transform.translate - fromPosition).Length();
					if (length < minLength) {
						minLength = length;
					}
				}
			}
		}
		return minLength;
		});

}
