#pragma once
#include "Object/Component/ComponentStrage.h"

class EntityManager final {
private:
	std::unordered_map<size_t, std::unique_ptr<IComponentStrage>> componentStrages;
	uint32_t nextEntityId_;
public:
	EntityManager() : nextEntityId_(0) {}
	uint32_t CreateEntity() {
		return nextEntityId_++;
	}

	template <typename T>
	void AddComponent(uint32_t id, const T& component) {
		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) == componentStrages.end()) {
			componentStrages[typeId] = std::make_unique<ComponentStrage<T>>();
		}
		auto& strage = static_cast<ComponentStrage<T>&>(*componentStrages[typeId]);
		strage.AddComponent(id, component);
	}

	template <typename T>
	T& GetComponent(uint32_t id) {
		size_t typeId = typeid(T).hash_code();
		if (componentStrages.find(typeId) != componentStrages.end()) {
			auto& strage = static_cast<ComponentStrage<T>&>(*componentStrages[typeId]);
			return strage.GetComponent(id);
		}
		throw std::runtime_error("Component not found");
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
};