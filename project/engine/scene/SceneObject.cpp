#include "SceneObject.h"
#include "SceneManager.h"

namespace {
	const std::string kEntityKey = "entities";
}

void QFE::SCENE::SceneObject::Initialize() {
	entityManager_.ResetEntity();
}

void QFE::SCENE::SceneObject::EndFrame() {
	entityManager_.EndFrame();
}

void QFE::SCENE::SceneObject::SaveSceneToJson(const std::string& filePath) {
	nlohmann::json sceneJson;
	// 配列として初期化する
	sceneJson[kEntityKey] = nlohmann::json::array();

	// EntityManagerの状態をJSONにシリアライズしてファイルに保存する
	std::vector<uint32_t> activeEntityIds = entityManager_.GetActiveEntityIds();
	std::sort(activeEntityIds.begin(), activeEntityIds.end());

	// 全アクティブエンティティをループ（EntityManagerの管理順、またはID順）
	for (uint32_t id : activeEntityIds) {

		// 1つのエンティティのコンポーネント群をシリアライズ
		nlohmann::json entityComponentsJson = entityManager_.SerializeEntityComponents(id);

		// 空っぽのエンティティでなければ、配列の末尾に追加（push_back）していく
		if (!entityComponentsJson.empty()) {
			sceneJson[kEntityKey].push_back(entityComponentsJson);
		}
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

	// JSONに "entities" キーが存在し、かつそれが配列であることを確認
	if (!sceneJson.contains(kEntityKey)) {
		QFE_LOG("Invalid scene JSON format: 'entities' key is missing.");
		return;
	}
	if (!sceneJson[kEntityKey].is_array()) {
		QFE_LOG("Invalid scene JSON format: 'entities' is not an array.");
		return;
	}
	// JSON配列を取得
	const auto& entitiesArrayJson = sceneJson[kEntityKey];

	// JSON配列のエンティティの数だけ、上から順番にループを回す
	for (const auto& componentsJson : entitiesArrayJson) {

		// 新しいエンティティを作成し、そのIDを取得
		uint32_t newEntityId = entityManager_.CreateEntity();
		// JSONからコンポーネントを復元する
		entityManager_.DeserializeEntityComponents(newEntityId, componentsJson);
	}
}

QFE::EntityManager& QFE::SCENE::SceneObject::GetEntityManager() {
	return entityManager_;
}
