#pragma once
#include "IComponentStrage.h"
#include <unordered_map>
#include <memory>

template <typename T>
class ComponentStrage : public IComponentStrage {
private:
	std::unordered_map<uint32_t, T> components;

public:
	void AddComponent(uint32_t id, const T& component) {
		components[id] = component;
	}
	T& GetComponent(uint32_t id) {
		auto it = components.find(id);
		if (it != components.end()) {
			return it->second;
		}
		throw std::runtime_error("Component not found");
	}
	void RemoveComponent(uint32_t id) {
		components.erase(id);
	}
	auto& GetAllComponents() {
		return components;
	}
	bool HasComponent(uint32_t id) const {
		return components.find(id) != components.end();
	}
};
