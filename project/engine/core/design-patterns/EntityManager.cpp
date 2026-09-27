#include "EntityManager.h"
#include "components/ObjectInfoComponent.h"
#include "components/ParentComponent.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace QFE;

QFE::EntityManager::EntityManager() : nextEntityId_(0) {
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        // 起動時に QFE_COMPONENT マクロで集まった全コンポーネントのストレージをここで一発生成
        componentStorages[entry.typeId] = entry.creator();
    }
}

void QFE::EntityManager::EndFrame() {
    for (uint32_t id : entitiesToRemove_) {
        for (auto& [typeId, storage] : componentStorages) {
            storage->RemoveComponent(id);
        }
        activeEntityIds_.erase(id);
    }
    entitiesToRemove_.clear();
}

nlohmann::json QFE::EntityManager::Serialize() const {
    nlohmann::json entitiesJson;
	uint32_t saveId = 0;
    for (uint32_t entityId : activeEntityIds_) {
        entitiesJson[saveId++] = SerializeEntityComponents(entityId);
    }
    return entitiesJson;
}

nlohmann::json QFE::EntityManager::SerializeEntityComponents(uint32_t entityId) const {
    nlohmann::json componentsJson;

    // 自動登録のエントリーを基準に回すことで、環境依存の typeid().name() を回避する
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        auto it = componentStorages.find(entry.typeId);
        if (it != componentStorages.end() && it->second->HasComponent(entityId)) {
            nlohmann::json compJson;
            JsonArchive archive(compJson, false);

            it->second->ReflectComponent(entityId, archive);
            componentsJson[entry.name] = compJson; // マクロで定義した綺麗な名前がJSONのキーになる
        }
    }
    return componentsJson;
}

void QFE::EntityManager::DeserializeEntityComponents(uint32_t entityId, const nlohmann::json& componentsJson) {
    // 自動登録されたコンポーネントを走査する。
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        // JSONの中に、このコンポーネントの名前があるか
        if (componentsJson.contains(entry.name)) {
            auto& storagePtr = componentStorages[entry.typeId];

            // もしエンティティにまだコンポーネントが割り当てられていなければ、ここでデフォルト構築
            if (!storagePtr->HasComponent(entityId)) {
                storagePtr->AddDefaultComponent(entityId);
            }

            // JSONからコンポーネントを復元
            nlohmann::json compJson = componentsJson[entry.name];
            JsonArchive archive(compJson, true);

            storagePtr->ReflectComponent(entityId, archive);
        }
    }
}

nlohmann::json QFE::EntityManager::SerializeComponent(uint32_t entityId, const std::string& componentTypeName) const {
	return SerializeEntityComponents(entityId)[componentTypeName];
}

std::vector<std::string> QFE::EntityManager::GetComponentTypeNames(uint32_t entityId) const {
    std::vector<std::string> componentNames;
    // 自動登録されたコンポーネントのエントリーを走査する
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        auto it = componentStorages.find(entry.typeId);
        if (it != componentStorages.end() && it->second->HasComponent(entityId)) {
            componentNames.push_back(entry.name); // マクロで定義した綺麗な名前を返す
        }
    }
	return componentNames;
}

std::vector<std::string> QFE::EntityManager::GetAllComponentTypeNames() const {
    std::vector<std::string> componentTypeNames;
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        componentTypeNames.push_back(entry.name);
    }
    return componentTypeNames;
}

void QFE::EntityManager::ResetEntity() {
    for (auto& [typeId, storage] : componentStorages) {
        storage->Clear();
    }
    activeEntityIds_.clear();
    nextEntityId_ = 0;
}

void QFE::EntityManager::InstantRemoveEntity(uint32_t id) {
	std::vector<uint32_t> entityIds{ id };
	const std::vector<uint32_t> descendants = GetDescendantEntityIds(id);
	entityIds.insert(entityIds.end(), descendants.begin(), descendants.end());

	for (const uint32_t entityId : entityIds) {
		for (auto& [typeId, storage] : componentStorages) {
			storage->RemoveComponent(entityId);
		}
		activeEntityIds_.erase(entityId);
	}

	entitiesToRemove_.erase(
		std::remove_if(entitiesToRemove_.begin(), entitiesToRemove_.end(),
			[&entityIds](uint32_t queuedId) {
				return std::find(entityIds.begin(), entityIds.end(), queuedId) != entityIds.end();
			}),
		entitiesToRemove_.end());
}

uint32_t QFE::EntityManager::CreateEntity() {
    uint32_t id = nextEntityId_++;
    activeEntityIds_.insert(id);
    return id;
}

bool QFE::EntityManager::ForceCreateEntity(uint32_t id) {
    if(activeEntityIds_.find(id) == activeEntityIds_.end()) {
        activeEntityIds_.insert(id);
        if (id >= nextEntityId_) {
            nextEntityId_ = id + 1;
        }
        return true;
	}
    return false;
}

void QFE::EntityManager::RemoveEntity(uint32_t id) {
	std::vector<uint32_t> entityIds{ id };
	const std::vector<uint32_t> descendants = GetDescendantEntityIds(id);
	entityIds.insert(entityIds.end(), descendants.begin(), descendants.end());

	for (const uint32_t entityId : entityIds) {
		if (std::find(entitiesToRemove_.begin(), entitiesToRemove_.end(), entityId) ==
			entitiesToRemove_.end()) {
			entitiesToRemove_.push_back(entityId);
		}
	}
}

void QFE::EntityManager::CancelEntityRemoval(uint32_t id) {
	entitiesToRemove_.erase(
		std::remove(entitiesToRemove_.begin(), entitiesToRemove_.end(), id),
		entitiesToRemove_.end());
}

std::vector<uint32_t> QFE::EntityManager::GetDescendantEntityIds(uint32_t id) const {
	std::vector<uint32_t> descendants;
	if (!IsActiveEntity(id) || !HasComponent<QFE::SCENE::ObjectInfoComponent>(id)) {
		return descendants;
	}

	const std::vector<uint32_t> activeEntityIds = GetActiveEntityIds();
	std::unordered_map<std::string, std::vector<uint32_t>> childrenByParentUuid;
	for (const uint32_t candidateId : activeEntityIds) {
		if (!HasComponent<QFE::SCENE::ParentComponent>(candidateId)) {
			continue;
		}
		const std::string& parentUuid =
			GetComponent<QFE::SCENE::ParentComponent>(candidateId).parent.uuid;
		if (!parentUuid.empty()) {
			childrenByParentUuid[parentUuid].push_back(candidateId);
		}
	}

	std::vector<uint32_t> pendingEntityIds{ id };
	std::unordered_set<uint32_t> visitedEntityIds{ id };

	for (size_t pendingIndex = 0; pendingIndex < pendingEntityIds.size(); ++pendingIndex) {
		const uint32_t parentEntityId = pendingEntityIds[pendingIndex];
		if (!HasComponent<QFE::SCENE::ObjectInfoComponent>(parentEntityId)) {
			continue;
		}
		const std::string& parentUuid =
			GetComponent<QFE::SCENE::ObjectInfoComponent>(parentEntityId).uuid;
		const auto childrenIt = childrenByParentUuid.find(parentUuid);
		if (parentUuid.empty() || childrenIt == childrenByParentUuid.end()) {
			continue;
		}

		for (const uint32_t candidateId : childrenIt->second) {
			if (visitedEntityIds.insert(candidateId).second) {
				descendants.push_back(candidateId);
				pendingEntityIds.push_back(candidateId);
			}
		}
	}

	return descendants;
}

bool QFE::EntityManager::IsActiveEntity(uint32_t id) const {
    return activeEntityIds_.find(id) != activeEntityIds_.end();
}

void* QFE::EntityManager::GetComponentRaw(uint32_t entityId, const char* componentTypeName) {

	// 自動登録されたコンポーネントのエントリーを走査する
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {

        // 引数で渡された名前と、マクロで登録された名前が一致するかチェック
        if (entry.name == componentTypeName) {

            // 一致したら、EXE側の正しいレイアウトのハッシュマップからストレージを探す
            auto it = componentStorages.find(entry.typeId);
            if (it != componentStorages.end()) {

                // 先ほど追加した GetRawPtr を使って、安全に生ポインタ（void*）を引き出して返す！
                return it->second->GetRawPtr(entityId);
            }
        }
    }

    return nullptr; // 見つからなければ安全にnullを返す
}

void QFE::EntityManager::RemoveComponent(uint32_t entityId, const char* componentTypeName) {
    // 自動登録されたコンポーネントのエントリーを走査する
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        // 引数で渡された名前と、マクロで登録された名前が一致するかチェック
        if (entry.name == componentTypeName) {
            // 一致したら、EXE側の正しいレイアウトのハッシュマップからストレージを探す
            auto it = componentStorages.find(entry.typeId);
            if (it != componentStorages.end()) {
                // ストレージが見つかったら、コンポーネントを削除する
                it->second->RemoveComponent(entityId);
                return; // 削除後はループを抜ける
            }
        }
	}
}

uint32_t QFE::EntityManager::GetNextEntityId() const {
    return nextEntityId_;
}

std::vector<uint32_t> QFE::EntityManager::GetActiveEntityIds() const {
    std::vector<uint32_t> sortedIds(activeEntityIds_.begin(), activeEntityIds_.end());
    std::sort(sortedIds.begin(), sortedIds.end());
    return sortedIds;
}

void QFE::EntityManager::AddDefaultComponent(uint32_t id, const std::string& componentTypeName) {
    // 自動登録のエントリーを走査
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        if (entry.name == componentTypeName) {
            auto it = componentStorages.find(entry.typeId);
            if (it != componentStorages.end()) {
                it->second->AddDefaultComponent(id);
            }
        }
    }
}

void QFE::EntityManager::DeleteComponent(uint32_t id, const std::string& componentTypeName) {
    // 自動登録のエントリーを走査
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        if (entry.name == componentTypeName) {
            auto it = componentStorages.find(entry.typeId);
            if (it != componentStorages.end()) {
                it->second->RemoveComponent(id);
            }
        }
	}
}

void QFE::EntityManager::ReflectionComponent(uint32_t id, Archive& ar) {
    // 自動登録されたコンポーネントを走査する。
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
		auto& storagePtr = componentStorages[entry.typeId];
        storagePtr->ReflectComponent(id, ar);
    }
}

void QFE::EntityManager::ReflectionComponentByName(uint32_t entityId, const std::string& componentTypeName, QFE::Archive& archive) {
    // 自動登録のエントリーを走査
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        if (entry.name == componentTypeName) {
            auto it = componentStorages.find(entry.typeId);
            if (it != componentStorages.end()) {
				it->second->ReflectComponent(entityId, archive);
            }
        }
    }
}
