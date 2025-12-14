#pragma once
#include "Component/ComponentStrage.h"
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>


class EntityManager final {
private:
	std::unordered_map<size_t, std::unique_ptr<IComponentStrage>> componentStrages;
	uint32_t nextEntityId_;
	std::unordered_set<uint32_t> activeEntityIds_;
	std::vector<uint32_t> entitiesToRemove_;

public:
	EntityManager() : nextEntityId_(0) {}
	nlohmann::json SerializeEntityComponents(uint32_t entityId) const {
		nlohmann::json componentsJson;
		for (const auto& [typeId, stragePtr] : componentStrages) {
			const ComponentData* comp = stragePtr->GetComponentDataPtr(entityId);
			if (comp) {
				componentsJson[comp->GetTypeName()] = comp->Serialize();
			}
		}
		return componentsJson;
	}

	void EndFrame() {
		// 削除予定�EEntityに紐づくコンポ�Eネントを全て削除
		for (uint32_t id : entitiesToRemove_) {
			for (auto& [typeId, strage] : componentStrages) {
				strage->RemoveComponent(id);
			}
			activeEntityIds_.erase(id);
		}
		entitiesToRemove_.clear();
	}

	void ResetEntiry() {
		componentStrages.clear();
		activeEntityIds_.clear();
		nextEntityId_ = 0;
	}

	void InstantRemoveEntity(uint32_t id) {
		for (auto& [typeId, strage] : componentStrages) {
			strage->RemoveComponent(id);
		}
		activeEntityIds_.erase(id);
	}

	uint32_t CreateEntity() {
		uint32_t id = nextEntityId_++;
		activeEntityIds_.insert(id);
		return id;
	}

	void RemoveEntity(uint32_t id) {
		entitiesToRemove_.push_back(id);
	}

	bool IsActiveEntity(uint32_t id) const {
		return activeEntityIds_.find(id) != activeEntityIds_.end();
	}

	template <typename T, typename... Args>
	void EmplaceComponent(uint32_t id, Args&&... args) {
		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) == componentStrages.end()) {
			componentStrages[typeId] = std::make_unique<ComponentStrage<T>>();
		}
		auto& strage = static_cast<ComponentStrage<T>&>(*componentStrages[typeId]);
		strage.AddComponent(id, T(std::forward<Args>(args)...));
	}

	template <typename T>
	T& GetComponent(uint32_t id) const {
		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) != componentStrages.end()) {
			auto& strage = static_cast<ComponentStrage<T>&>(*componentStrages.at(typeId));
			return strage.GetComponent(id);
		}
		throw std::runtime_error("Component not found");
	}
	template <typename T>
	T* GetComponentPtr(uint32_t id) const {
		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) != componentStrages.end()) {
			auto& strage = static_cast<ComponentStrage<T>&>(*componentStrages.at(typeId));
			return strage.GetComponentPtr(id);
		}
		assert(false && "Component strage not found");
		return nullptr;
	}

	template <typename T>
	void RemoveComponent(uint32_t id) {
		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) != componentStrages.end()) {
			auto& strage = static_cast<ComponentStrage<T>&>(*componentStrages[typeId]);
			strage.RemoveComponent(id);
		}
	}

	template <typename T>
	const ComponentStrage<T>& GetComponentStrage() const {
		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) != componentStrages.end()) {
			return static_cast<const ComponentStrage<T>&>(*componentStrages.at(typeId));
		}
		throw std::runtime_error("Component strage not found");
	}

	template <typename T>
	bool HasComponentStrage() const {
		size_t typeId = typeid(T).hash_code();
		return componentStrages.find(typeId) != componentStrages.end();
	}

	template <typename T>
	bool HasComponent(uint32_t id) const {
		if (id >= nextEntityId_) {
			return false;
		}

		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) != componentStrages.end()) {
			const auto& strage = static_cast<const ComponentStrage<T>&>(*componentStrages.at(typeId));
			return strage.HasComponent(id);
		}
		return false;
	}

	uint32_t GetNextEntityId() const {
		return nextEntityId_;
	}

	std::vector<uint32_t> GetActiveEntityIds() const {
		return std::vector<uint32_t>(activeEntityIds_.begin(), activeEntityIds_.end());
	}
};
