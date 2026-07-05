#pragma once
#include "IComponentStorage.h"
#include "../../memory/SparseSets.h"
#include <memory>
#include <stdexcept>

namespace QFE {
	/// @brief コンポーネントのストレージクラス
	template <typename T>
	class ComponentStorage : public IComponentStorage {
	private:
		SparseSet<T> components;

	public:
		ComponentStorage() = default;

		auto begin() const { return components.begin(); }
		auto end() const { return components.end(); }
		auto begin() { return components.begin(); }
		auto end() { return components.end(); }
		auto size() const { return components.size(); }

	public:
		/// @brief コンポーネントを追加する。既に存在する場合は上書きする。
		void AddComponent(uint32_t id, const T& component) {
			components[id] = component;
		}
		T& GetComponent(uint32_t id) {
			T* ptr = components.find(id);
			if (ptr) {
				return *ptr;
			}
			throw std::runtime_error("Component not found");
		}
		T* GetComponentPtr(uint32_t id) {
			return components.find(id);
		}
		void RemoveComponent(uint32_t id) override {
			components.erase(id);
		}
		auto& GetAllComponents() {
			return components;
		}
		bool HasComponent(uint32_t id) const {
			return components.Contains(id);
		}
		ComponentData* GetComponentDataPtr(uint32_t id) override {
			return components.find(id);
		}

		/// @brief 全ての有効なコンポーネントに対して関数を実行する。
		void Each(const std::function<void(uint32_t, T&)>& func) {
			components.Each(func);
		}
		/// @brief 全ての有効なコンポーネントに対して関数を実行する（const版）。
		void Each(const std::function<void(uint32_t, const T&)>& func) const {
			components.Each(func);
		}

		/// @brief コンポーネントIDの一覧を取得する。
		std::vector<uint32_t> GetEntityIds() const {
			return components.Keys();
		}
		/// @brief コンポーネントの一覧を取得する。
		std::vector<T> GetComponents() const {
			return components.Values();
		}
	};

}
