#include "Engine/include/scene/SceneCommand/RemakeUniqeIDCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/utility/ID/UniqeIDManager.h"

#include "engine/include/scene/Data/SceneObjectData.h"

RemakeUniqeIDCommand::RemakeUniqeIDCommand(EntityManager& entityManager, ListUniqueIDManager& uidManager) 
	: ISceneEntityCommand(entityManager) , uniqueIdManager_(uidManager){}

void RemakeUniqeIDCommand::Execute() {
	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<SceneObjectData>(entityId)) {
			SceneObjectData& sceneObjectData = entityManager_.GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.uniqueId == 0) {
				sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
			}
			else {
				uniqueIdManager_.AddUsedID(sceneObjectData.uniqueId);
			}
		}
	}
	// 重複してたら新しいID振る
	std::set<uint32_t> checkIds;
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<SceneObjectData>(entityId)) {
			SceneObjectData& sceneObjectData = entityManager_.GetComponent<SceneObjectData>(entityId);
			if (checkIds.find(sceneObjectData.uniqueId) != checkIds.end()) {
				sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
			}
			checkIds.insert(sceneObjectData.uniqueId);
		}
	}
}

void RemakeUniqeIDCommand::Undo() {
	// ユニークIDの再生成は元に戻せない
}

std::string RemakeUniqeIDCommand::GetCommandName() const {
	return "Remake Unique ID Command";
}
