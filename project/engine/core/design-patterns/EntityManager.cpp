#include "EntityManager.h"

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
		for (auto& [name, storage] : dynamicComponentStorages_) {
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
	for (const auto& [name, storage] : dynamicComponentStorages_) {
		if (storage->HasComponent(entityId)) {
			nlohmann::json compJson;
			JsonArchive archive(compJson, false);
			storage->ReflectComponent(entityId, archive);
			componentsJson[name] = compJson;
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
	for (const auto& [name, storage] : dynamicComponentStorages_) {
		if (!componentsJson.contains(name)) {
			continue;
		}
		if (!storage->HasComponent(entityId)) {
			storage->AddDefaultComponent(entityId);
		}
		nlohmann::json compJson = componentsJson[name];
		JsonArchive archive(compJson, true);
		storage->ReflectComponent(entityId, archive);
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
	for (const auto& [name, storage] : dynamicComponentStorages_) {
		if (storage->HasComponent(entityId)) {
			componentNames.push_back(name);
		}
	}
	return componentNames;
}

std::vector<std::string> QFE::EntityManager::GetAllComponentTypeNames() const {
    std::vector<std::string> componentTypeNames;
    for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
        componentTypeNames.push_back(entry.name);
    }
	for (const auto& [name, storage] : dynamicComponentStorages_) {
		(void)storage;
		componentTypeNames.push_back(name);
	}
    return componentTypeNames;
}

bool QFE::EntityManager::RegisterDynamicComponent(const DynamicComponentDescriptor& descriptor) {
	if (descriptor.stableName == nullptr || descriptor.stableName[0] == '\0' ||
		dynamicComponentStorages_.contains(descriptor.stableName)) {
		return false;
	}
	try {
		dynamicComponentStorages_.emplace(
			descriptor.stableName,
			std::make_unique<DynamicComponentStorage>(descriptor));
		return true;
	} catch (...) {
		return false;
	}
}

bool QFE::EntityManager::UnregisterDynamicComponent(const std::string& componentTypeName) {
	auto it = dynamicComponentStorages_.find(componentTypeName);
	if (it == dynamicComponentStorages_.end()) {
		return false;
	}
	// erase時にDynamicComponentStorageのデストラクタがDLL側destroyを呼ぶ。
	// DLLがロードされている間に実行する必要がある。
	dynamicComponentStorages_.erase(it);
	return true;
}

bool QFE::EntityManager::IsDynamicComponentType(const std::string& componentTypeName) const {
	return dynamicComponentStorages_.contains(componentTypeName);
}

void QFE::EntityManager::UpdateDynamicComponents(float deltaTime) {
	for (auto& [name, storage] : dynamicComponentStorages_) {
		(void)name;
		for (uint32_t entityId : activeEntityIds_) {
			if (storage->HasComponent(entityId)) {
				storage->Update(entityId, deltaTime, this);
			}
		}
	}
}

void QFE::EntityManager::ResetEntity() {
    for (auto& [typeId, storage] : componentStorages) {
        storage->Clear();
    }
	for (auto& [name, storage] : dynamicComponentStorages_) {
		storage->Clear();
	}
    activeEntityIds_.clear();
    nextEntityId_ = 0;
}

void QFE::EntityManager::InstantRemoveEntity(uint32_t id) {
    for (auto& [typeId, storage] : componentStorages) {
        storage->RemoveComponent(id);
    }
	for (auto& [name, storage] : dynamicComponentStorages_) {
		storage->RemoveComponent(id);
	}
    activeEntityIds_.erase(id);
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
    entitiesToRemove_.push_back(id);
}

bool QFE::EntityManager::IsActiveEntity(uint32_t id) const {
    return activeEntityIds_.find(id) != activeEntityIds_.end();
}

void* QFE::EntityManager::GetComponentRaw(uint32_t entityId, const char* componentTypeName) {
	if (componentTypeName == nullptr) {
		return nullptr;
	}
	auto dynamicIt = dynamicComponentStorages_.find(componentTypeName);
	if (dynamicIt != dynamicComponentStorages_.end()) {
		return dynamicIt->second->GetRawPtr(entityId);
	}

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
	if (componentTypeName == nullptr) {
		return;
	}
	auto dynamicIt = dynamicComponentStorages_.find(componentTypeName);
	if (dynamicIt != dynamicComponentStorages_.end()) {
		dynamicIt->second->RemoveComponent(entityId);
		return;
	}
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
	auto dynamicIt = dynamicComponentStorages_.find(componentTypeName);
	if (dynamicIt != dynamicComponentStorages_.end()) {
		dynamicIt->second->AddDefaultComponent(id);
		return;
	}
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
	auto dynamicIt = dynamicComponentStorages_.find(componentTypeName);
	if (dynamicIt != dynamicComponentStorages_.end()) {
		dynamicIt->second->RemoveComponent(id);
		return;
	}
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
	for (auto& [name, storage] : dynamicComponentStorages_) {
		(void)name;
		storage->ReflectComponent(id, ar);
	}
}

void QFE::EntityManager::ReflectionComponentByName(uint32_t entityId, const std::string& componentTypeName, QFE::Archive& archive) {
	auto dynamicIt = dynamicComponentStorages_.find(componentTypeName);
	if (dynamicIt != dynamicComponentStorages_.end()) {
		dynamicIt->second->ReflectComponent(entityId, archive);
		return;
	}
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
