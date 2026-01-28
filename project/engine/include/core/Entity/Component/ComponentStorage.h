#pragma once
#include "IComponentStorage.h"
#include <unordered_map>
#include <memory>
#include <stdexcept>

namespace QFE {

	template <typename T>
	class ComponentStorage : public IComponentStorage {
	private:
		std::unordered_map<uint32_t, T> components;

	public:
		auto begin() const { return components.begin(); }
		auto end() const { return components.end(); }
		auto begin() { return components.begin(); }
		auto end() { return components.end(); }
		auto size() const { return components.size(); }

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
		T* GetComponentPtr(uint32_t id) {
			auto it = components.find(id);
			if (it != components.end()) {
				return &(it->second);
			}
			return nullptr;
		}
		void RemoveComponent(uint32_t id) override {
			components.erase(id);
		}
		auto& GetAllComponents() {
			return components;
		}
		bool HasComponent(uint32_t id) const {
			return components.find(id) != components.end();
		}
		ComponentData* GetComponentDataPtr(uint32_t id) override {
			auto it = components.find(id);
			if (it != components.end()) {
				return &(it->second);
			}
			return nullptr;
		}
	};

}
