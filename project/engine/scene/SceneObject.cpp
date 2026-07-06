#include "SceneObject.h"
#include "SceneManager.h"

void QFE::SCENE::SceneObject::Initialize() {
	entityManager_.ResetEntity();
}

void QFE::SCENE::SceneObject::EndFrame() {
	entityManager_.EndFrame();
}

void QFE::SCENE::SceneObject::SaveSceneToJson(const std::string& filePath) {
	// シーンの情報
	nlohmann::json sceneJson;

	// EntityManagerの状態をJSONにシリアライズしてファイルに保存する
	std::vector<uint32_t> activeEntityIds = entityManager_.GetActiveEntityIds();
	sceneJson["entity_count"] = activeEntityIds.size();
	uint32_t key = 0;
	for(uint32_t entityId : activeEntityIds) {
		// エンティティIDは破棄して順番に書き込む
		sceneJson["entities"][std::to_string(key++)] = entityManager_.SerializeEntityComponents(entityId);
	}

	// JSONをファイルに保存
	std::ofstream file(filePath);
	if (!file.is_open()) {
		QFE_REPORT_SYSTEM_ERROR("Failed to open file for saving scene: " + filePath, SystemError::Abort);
		return;
	}
	file << sceneJson.dump(4); // インデント付きで保存
	return;
}

void QFE::SCENE::SceneObject::LoadSceneFromJson(const std::string& filePath) {
	// JSONファイルを読み込む
	std::ifstream file(filePath);
	if (!file.is_open()) {
		QFE_REPORT_SYSTEM_ERROR("Failed to open file for loading scene: " + filePath, SystemError::Abort);
		return;
	}
	nlohmann::json sceneJson;
	file >> sceneJson;

	// EntityManagerをリセットしてからロードする
	entityManager_.ResetEntity();
	// エンティティの数を取得
	size_t entityCount = sceneJson["entity_count"].get<size_t>();
	// エンティティを順番に復元する
	for (size_t i = 0; i < entityCount; ++i) {
		uint32_t newEntityId = entityManager_.CreateEntity();
		const nlohmann::json& entityComponentsJson = sceneJson["entities"][std::to_string(i)];
		entityManager_.DeserializeEntityComponents(newEntityId, entityComponentsJson);
	}
}

QFE::EntityManager& QFE::SCENE::SceneObject::GetEntityManager() {
	return entityManager_;
}
