#pragma once
#include "component/ComponentStorage.h"
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>
#include "component/JsonArchive.h"
#include "component/ComponentAutoRegistry.h" // 追加

namespace QFE {
	class EntityManager final {
	private:
		std::unordered_map<size_t, std::unique_ptr<IComponentStorage>> componentStorages;
		uint32_t nextEntityId_;
		std::unordered_set<uint32_t> activeEntityIds_;
		std::vector<uint32_t> entitiesToRemove_;

	public:
		// ★コンストラクタを修正：初期化時にすべてのストレージを全自動で生成して登録する！
		EntityManager() : nextEntityId_(0) {
			for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
				// 起動時に QFE_COMPONENT マクロで集まった全コンポーネントのストレージをここで一発生成
				componentStorages[entry.typeId] = entry.creator();
			}
		}

		void EndFrame() {
			for (uint32_t id : entitiesToRemove_) {
				for (auto& [typeId, storage] : componentStorages) {
					storage->RemoveComponent(id);
				}
				activeEntityIds_.erase(id);
			}
			entitiesToRemove_.clear();
		}

		// ★シリアライズ側も自動登録の名前を使うように修正
		nlohmann::json SerializeEntityComponents(uint32_t entityId) const {
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

		// ★デシリアライズ側：初期化時にすべてのストレージが確実に存在しているので、JSONにあるキーだけを狙い撃ちで復元できる
		void DeserializeEntityComponents(uint32_t entityId, const nlohmann::json& componentsJson) {
			for (const auto& entry : ComponentAutoRegistry::Instance().GetEntries()) {
				// JSONの中に、このコンポーネントの名前（例: "TransformComponent"）があるか？
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

		void ResetEntity() {
			for(auto& [typeId, storage] : componentStorages) {
				storage->Clear();
			}
			activeEntityIds_.clear();
			nextEntityId_ = 0;
		}
		void InstantRemoveEntity(uint32_t id) {
			for (auto& [typeId, storage] : componentStorages) {
				storage->RemoveComponent(id);
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

		template <typename T>
		bool HasComponentStrage() const {
			size_t typeId = typeid(T).hash_code();
			return componentStorages.find(typeId) != componentStorages.end();
		}
		template <typename T>
		bool HasComponent(uint32_t id) const {
			if (id >= nextEntityId_) return false;
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				const auto& storage = static_cast<const ComponentStorage<T>&>(*componentStorages.at(typeId));
				return storage.HasComponent(id);
			}
			return false;
		}

		template <typename T, typename... Args>
		void EmplaceComponent(uint32_t id, Args&&... args) {
			size_t typeId = typeid(T).hash_code();
			// 初期化時点で必ず生成されているため、ここの if 文すら本来は通過しません（より安全・高速化）
			if (componentStorages.find(typeId) == componentStorages.end()) {
				componentStorages[typeId] = std::make_unique<ComponentStorage<T>>();
			}
			auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages[typeId]);
			storage.AddComponent(id, T(std::forward<Args>(args)...));
		}
		template <typename T>
		T& GetComponent(uint32_t id) const {
			size_t typeId = typeid(T).hash_code();
			if (HasComponent<T>(id)) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
				return storage.GetComponent(id);
			}
			throw std::runtime_error("Component not found");
		}
		template <typename T>
		T* GetComponentPtr(uint32_t id) const {
			size_t typeId = typeid(T).hash_code();
			if (HasComponent<T>(id)) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
				return storage.GetComponentPtr(id);
			}
			assert(false && "Component strage not found");
			return nullptr;
		}
		template <typename T>
		void RemoveComponent(uint32_t id) {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages[typeId]);
				storage.RemoveComponent(id);
			}
		}
		template <typename T>
		ComponentStorage<T>& GetComponentStrage() const {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				return static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
			}
			assert(false && "Component strage not found");
			return *static_cast<ComponentStorage<T>*>(nullptr);
		}

		template <typename T>
		void Each(const std::function<void(uint32_t, T&)>& func) {
			if (HasComponentStrage<T>()) {
				auto& storage = GetComponentStrage<T>();
				storage.Each(func);
			}
		}
		template <typename T>
		void Each(const std::function<void(uint32_t, const T&)>& func) const {
			if (HasComponentStrage<T>()) {
				const auto& storage = GetComponentStrage<T>();
				storage.Each(func);
			}
		}

		uint32_t GetNextEntityId() const {
			return nextEntityId_;
		}
		std::vector<uint32_t> GetActiveEntityIds() const {
			std::vector<uint32_t> sortedIds(activeEntityIds_.begin(), activeEntityIds_.end());
			std::sort(sortedIds.begin(), sortedIds.end());
			return sortedIds;
		}
	};
}