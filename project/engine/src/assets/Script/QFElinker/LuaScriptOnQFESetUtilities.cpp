#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetUtilities.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/assets/Script/LuaScriptExecutor.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include "engine/include/utility/FileSystems/FileUtility.h"

void QFE::Script::Utility::LuaScriptOnQFESetUtility(sol::state* luaState, EntityManager* entityManager, LuaScriptExecutor* luaScriptExecutor) {

	// CSV読み込み
	luaState->set_function("Load2DMap", [](const std::string& fileName) {
		std::vector<std::vector<uint32_t>> result;
		std::string path = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("2DMap") + fileName;
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Load2DMap: " + path);
#endif // QFE_OPTIMIZE_OFF

		if (QFE::FILE::LoadCSVToVector(path, result)) {
			return sol::as_table(result);
		}
		else {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Failed to load 2D map: " + path, LogLevel::Error);
#endif // QFE_OPTIMIZE_OFF
			return sol::as_table(std::vector<std::vector<uint32_t>>{});
		}
		});

	luaState->set_function("CountEntityName", [entityManager](const std::string& entityName) {
		int32_t count = 0;
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

	luaState->set_function("CountEntityTag", [entityManager](const std::string& entityTag) {
		int32_t count = 0;
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

	luaState->set_function("RunAllFunction", [luaScriptExecutor](const std::string& message) {
		luaScriptExecutor->RunFunctionIfExists(message);
		});

	luaState->set_function("GetMinLengthToEntityFromTag", [entityManager](const std::string& entityTag, const Vector3& fromPosition) {
		float minLength = FLT_MAX;
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
