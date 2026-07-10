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
        activeEntityIds_.erase(id);
    }
    entitiesToRemove_.clear();
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

void QFE::EntityManager::ResetEntity() {
    for (auto& [typeId, storage] : componentStorages) {
        storage->Clear();
    }
    activeEntityIds_.clear();
    nextEntityId_ = 0;
}

void QFE::EntityManager::InstantRemoveEntity(uint32_t id) {
    for (auto& [typeId, storage] : componentStorages) {
        storage->RemoveComponent(id);
    }
    activeEntityIds_.erase(id);
}

uint32_t QFE::EntityManager::CreateEntity() {
    uint32_t id = nextEntityId_++;
    activeEntityIds_.insert(id);
    return id;
}

void QFE::EntityManager::RemoveEntity(uint32_t id) {
    entitiesToRemove_.push_back(id);
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
