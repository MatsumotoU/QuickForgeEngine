#include "EntityManager.h"
#include "components/ObjectInfoComponent.h"
#include "components/ParentComponent.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace QFE;

QFE::EntityManager::EntityManager(std::weak_ptr<const StaticComponentRegistry> registry) :
	nextEntityId_(0), registry_(std::move(registry)) {
	CreateComponentStorages();
}

void QFE::EntityManager::CreateComponentStorages() {
	const std::shared_ptr<const StaticComponentRegistry> lockedRegistry = LockRegistry();
	if (!lockedRegistry->IsFrozen()) {
		throw std::runtime_error("Component registry must be frozen before EntityManager construction");
	}
    for(const auto& descriptor : lockedRegistry->GetComponentDescriptors()) {
		// ストレージインデックスの整合性を確認する
        if(descriptor.storageIndex != componentStorages.size()) {
			throw std::runtime_error("Component storage index mismatch");
        }
        // 静的レジストリに登録されたコンポーネントのストレージを生成
		std::unique_ptr<IComponentStorage> storage = descriptor.createStorageFunc();
		if (!storage) {
			throw std::runtime_error("Component storage factory returned null");
		}
		componentStorages.push_back(std::move(storage));
	}
}

void QFE::EntityManager::ReleaseComponentRegistryForReload() {
	// Storageの仮想関数とデストラクタはGameModule DLL内に存在し得るため、
	// DLLをアンロードする前に必ず破棄する。
	componentStorages.clear();
	registry_.reset();
	activeEntityIds_.clear();
	entitiesToRemove_.clear();
	nextEntityId_ = 0;
}

void QFE::EntityManager::BindComponentRegistryAfterReload(
	std::weak_ptr<const StaticComponentRegistry> registry) {
	if (!componentStorages.empty()) {
		throw std::runtime_error("Component storages must be released before rebinding the registry");
	}
	registry_ = std::move(registry);
	CreateComponentStorages();
}

std::shared_ptr<const QFE::StaticComponentRegistry> QFE::EntityManager::LockRegistry() const {
	std::shared_ptr<const StaticComponentRegistry> registry = registry_.lock();
	if (!registry) {
		throw std::runtime_error("Component registry lifetime ended before EntityManager");
	}
	return registry;
}

void QFE::EntityManager::EndFrame() {
	// 削除予定のエンティティを削除する
    for (uint32_t id : entitiesToRemove_) {
		// 子孫エンティティも含めて削除する
        for(auto& storage : componentStorages) {
            storage->RemoveComponent(id);
		}
		// 有効なエンティティIDのセットから削除する
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
	const std::shared_ptr<const StaticComponentRegistry> registry = LockRegistry();
	for(const ComponentDescriptor& descriptor : registry->GetComponentDescriptors()) {
		if (descriptor.storageIndex >= componentStorages.size()) {
			continue;
		}
		const auto& storage = componentStorages[descriptor.storageIndex];
        if (storage->HasComponent(entityId)) {
            nlohmann::json compJson;
            JsonArchive archive(compJson, false);
            storage->ReflectComponent(entityId, archive);
			componentsJson.emplace(descriptor.key, std::move(compJson));
        }
	}
    return componentsJson;
}

void QFE::EntityManager::DeserializeEntityComponents(uint32_t entityId, const nlohmann::json& componentsJson) {
	const std::shared_ptr<const StaticComponentRegistry> registry = LockRegistry();
	for (auto component = componentsJson.begin(); component != componentsJson.end(); ++component) {
		const uint32_t storageIndex = registry->GetComponentStorageIndex(component.key());
		if (storageIndex == StaticComponentRegistry::InvalidStorageIndex ||
			storageIndex >= componentStorages.size()) {
			continue;
		}

		auto& storage = componentStorages[storageIndex];
		if (!storage->HasComponent(entityId)) {
			storage->AddDefaultComponent(entityId);
		}

		nlohmann::json componentJson = component.value();
		JsonArchive archive(componentJson, true);
		storage->ReflectComponent(entityId, archive);
	}
}

nlohmann::json QFE::EntityManager::SerializeComponent(uint32_t entityId, const std::string& componentTypeName) const {
	const nlohmann::json componentsJson = SerializeEntityComponents(entityId);
	return componentsJson.at(componentTypeName);
}

std::vector<std::string> QFE::EntityManager::GetComponentTypeNames(uint32_t entityId) const {
	std::vector<std::string> componentNames;
	const std::shared_ptr<const StaticComponentRegistry> registry = LockRegistry();
	for (const ComponentDescriptor& descriptor : registry->GetComponentDescriptors()) {
		if (descriptor.storageIndex < componentStorages.size() &&
			componentStorages[descriptor.storageIndex]->HasComponent(entityId)) {
			componentNames.push_back(descriptor.key);
		}
	}
	return componentNames;
}

std::vector<std::string> QFE::EntityManager::GetAllComponentTypeNames() const {
	std::vector<std::string> componentNames;
	const std::shared_ptr<const StaticComponentRegistry> registry = LockRegistry();
	componentNames.reserve(registry->GetComponentDescriptors().size());
	for (const ComponentDescriptor& descriptor : registry->GetComponentDescriptors()) {
		componentNames.push_back(descriptor.key);
	}
	return componentNames;
}

void* QFE::EntityManager::GetComponentRaw(uint32_t entityId, const char* componentTypeName) {
	// 静的レジストリから、コンポーネント型名に対応するストレージインデックスを取得する
	const uint32_t storageIndex = LockRegistry()->GetComponentStorageIndex(componentTypeName);
	// ストレージインデックスが見つからなければ、nullptrを返す
    if(storageIndex == StaticComponentRegistry::InvalidStorageIndex) {
        return nullptr;
	}
	// ストレージインデックスが範囲外であれば、nullptrを返す
    if (componentStorages.size() <= storageIndex) {
        return nullptr;
    }
	// ストレージにコンポーネントが存在するか確認し、存在すれば生ポインタを返す
    if(componentStorages[storageIndex]->HasComponent(entityId)) {
        return componentStorages[storageIndex]->GetRawPtr(entityId);
	}
	return nullptr;
}

void QFE::EntityManager::RemoveComponent(uint32_t entityId, const char* componentTypeName) {
	// 静的レジストリから、コンポーネント型名に対応するストレージインデックスを取得する
	const uint32_t storageIndex = LockRegistry()->GetComponentStorageIndex(componentTypeName);

	// ストレージインデックスが見つからなければ、何もしない
    if (storageIndex == StaticComponentRegistry::InvalidStorageIndex) {
        return;
	}

    // ストレージインデックスが範囲外であれば、何もしない
    if (componentStorages.size() <= storageIndex) {
        return;
	}

    if (componentStorages[storageIndex]->HasComponent(entityId)) {
        componentStorages[storageIndex]->RemoveComponent(entityId);
    }
}

void QFE::EntityManager::ResetEntity() {
    for(auto& storage : componentStorages) {
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
		for (auto& storage : componentStorages) {
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

uint32_t QFE::EntityManager::GetNextEntityId() const {
    return nextEntityId_;
}

std::vector<uint32_t> QFE::EntityManager::GetActiveEntityIds() const {
    std::vector<uint32_t> sortedIds(activeEntityIds_.begin(), activeEntityIds_.end());
    std::sort(sortedIds.begin(), sortedIds.end());
    return sortedIds;
}

void QFE::EntityManager::AddDefaultComponent(uint32_t id, const std::string& componentTypeName) {
	const uint32_t storageIndex = LockRegistry()->GetComponentStorageIndex(componentTypeName);
	if (storageIndex != StaticComponentRegistry::InvalidStorageIndex &&
		storageIndex < componentStorages.size()) {
		componentStorages[storageIndex]->AddDefaultComponent(id);
    }
}

void QFE::EntityManager::DeleteComponent(uint32_t id, const std::string& componentTypeName) {
	RemoveComponent(id, componentTypeName.c_str());
}

void QFE::EntityManager::ReflectionComponent(uint32_t id, Archive& ar) {
	for (auto& storage : componentStorages) {
		storage->ReflectComponent(id, ar);
    }
}

void QFE::EntityManager::ReflectionComponentByName(uint32_t entityId, const std::string& componentTypeName, QFE::Archive& archive) {
	const uint32_t storageIndex = LockRegistry()->GetComponentStorageIndex(componentTypeName);
	if (storageIndex != StaticComponentRegistry::InvalidStorageIndex &&
		storageIndex < componentStorages.size()) {
		componentStorages[storageIndex]->ReflectComponent(entityId, archive);
    }
}
