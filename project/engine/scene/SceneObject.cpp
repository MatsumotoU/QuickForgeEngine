#include "SceneObject.h"
#include "SceneManager.h"
#include "file/FileUtility.h"
#include "design-patterns/component/EntityUuid.h"

#include <unordered_map>
#include <utility>
#include <vector>

namespace {
	const std::string kEntityKey = "entities";
	const std::string kEntityIdKey = "id";
	const std::string kComponentsKey = "components";
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
	std::filesystem::path osPath(filePath);
	std::ofstream file(osPath);
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
	for (const auto& entityJson : entitiesArrayJson) {
		// 新形式ではIDを保存して親参照を維持する。旧形式も引き続き読み込める。
		if (entityJson.contains(kEntityIdKey) && entityJson.contains(kComponentsKey)) {
			const uint32_t entityId = entityJson[kEntityIdKey].get<uint32_t>();
			if (entityManager_.ForceCreateEntity(entityId)) {
				entityManager_.DeserializeEntityComponents(entityId, entityJson[kComponentsKey]);
			}
		} else {
			const uint32_t newEntityId = entityManager_.CreateEntity();
			entityManager_.DeserializeEntityComponents(newEntityId, entityJson);
		}
	}
}

uint32_t QFE::SCENE::SceneObject::LoadEntityFromJsonObject(const std::string& filePath) {
	// ファイルが更新されていた場合はキャッシュを読み直す。
	std::error_code timestampError;
	const std::filesystem::file_time_type lastWriteTime =
		std::filesystem::last_write_time(filePath, timestampError);
	const auto cachedJson = objectJsonMap_.find(filePath);
	const auto cachedWriteTime = objectJsonLastWriteTimeMap_.find(filePath);
	const bool cacheNeedsReload = cachedJson == objectJsonMap_.end() || timestampError ||
		cachedWriteTime == objectJsonLastWriteTimeMap_.end() ||
		cachedWriteTime->second != lastWriteTime;
	if (cacheNeedsReload) {
		nlohmann::json loadedJson;
		if (!QFE::FILE::LoadFileToJson(filePath, loadedJson)) {
			objectJsonMap_.erase(filePath);
			objectJsonLastWriteTimeMap_.erase(filePath);
			QFE_LOG("Failed to load JSON file: " + filePath);
			return UINT32_MAX;
		}
		objectJsonMap_[filePath] = std::move(loadedJson);
		if (timestampError) {
			objectJsonLastWriteTimeMap_.erase(filePath);
		} else {
			objectJsonLastWriteTimeMap_[filePath] = lastWriteTime;
		}
	}

	const nlohmann::json& savedJson = objectJsonMap_[filePath];
	std::vector<nlohmann::json> componentSets;
	auto appendComponents = [&componentSets](const nlohmann::json& entityJson) {
		if (!entityJson.is_object()) {
			return;
		}

		const auto componentsIt = entityJson.find(kComponentsKey);
		const nlohmann::json& components =
			componentsIt != entityJson.end() && componentsIt->is_object()
			? *componentsIt
			: entityJson;
		if (!components.empty()) {
			componentSets.push_back(components);
		}
	};

	// Save Selected Entities とシーン保存形式の配列に対応する。
	const auto entitiesIt = savedJson.is_object() ? savedJson.find(kEntityKey) : savedJson.end();
	if (entitiesIt != savedJson.end() && entitiesIt->is_array()) {
		for (const nlohmann::json& entityJson : *entitiesIt) {
			appendComponents(entityJson);
		}
	} else {
		// 以前の単体エンティティJSON形式も読み込めるようにする。
		appendComponents(savedJson);
	}

	if (componentSets.empty()) {
		QFE_LOG("No entity components found in JSON file: " + filePath);
		return UINT32_MAX;
	}

	// 保存ファイルを何度追加してもUUIDが重複しないようにし、
	// 同時に読み込むエンティティ間の親子参照は新しいUUIDへ付け替える。
	std::unordered_map<std::string, std::string> uuidRemapping;
	for (nlohmann::json& components : componentSets) {
		auto infoIt = components.find("ObjectInfoComponent");
		if (infoIt == components.end() || !infoIt->is_object()) {
			continue;
		}
		auto uuidIt = infoIt->find("uuid");
		if (uuidIt == infoIt->end() || !uuidIt->is_string()) {
			continue;
		}

		const std::string oldUuid = uuidIt->get<std::string>();
		const std::string newUuid = QFE::GenerateEntityUuid();
		if (!oldUuid.empty()) {
			uuidRemapping[oldUuid] = newUuid;
		}
		*uuidIt = newUuid;
	}

	uint32_t firstEntityId = UINT32_MAX;
	for (nlohmann::json& components : componentSets) {
		auto parentComponentIt = components.find("ParentComponent");
		if (parentComponentIt != components.end() && parentComponentIt->is_object()) {
			auto parentIt = parentComponentIt->find("parent");
			if (parentIt != parentComponentIt->end()) {
				if (parentIt->is_string()) {
					auto remapped = uuidRemapping.find(parentIt->get<std::string>());
					if (remapped != uuidRemapping.end()) {
						*parentIt = remapped->second;
					}
				} else if (parentIt->is_object()) {
					auto parentUuidIt = parentIt->find("uuid");
					if (parentUuidIt != parentIt->end() && parentUuidIt->is_string()) {
						auto remapped = uuidRemapping.find(parentUuidIt->get<std::string>());
						if (remapped != uuidRemapping.end()) {
							*parentUuidIt = remapped->second;
						}
					}
				}
			}
		}

		// 追加なので保存時のEntity IDは再利用せず、現在のシーンで新規作成する。
		const uint32_t entityId = entityManager_.CreateEntity();
		entityManager_.DeserializeEntityComponents(entityId, components);
		if (firstEntityId == UINT32_MAX) {
			firstEntityId = entityId;
		}
	}

	return firstEntityId;
}

QFE::EntityManager& QFE::SCENE::SceneObject::GetEntityManager() {
	return entityManager_;
}
