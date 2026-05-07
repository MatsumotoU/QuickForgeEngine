#pragma once
#include "Component/ComponentStorage.h"
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>


namespace QFE {
	/// @brief エンティティ管理クラス
	class EntityManager final {
	private:
		// コンポーネントストレージ群
		std::unordered_map<size_t, std::unique_ptr<IComponentStorage>> componentStorages;
		// 次回生成エンティティID
		uint32_t nextEntityId_;
		// アクティブなエンティティID群
		std::unordered_set<uint32_t> activeEntityIds_;
		// 削除予定エンティティID群
		std::vector<uint32_t> entitiesToRemove_;

	public:
		EntityManager() : nextEntityId_(0) {}
		/// @brief 指定エンティティに紐づく全コンポーネントをシリアライズ
		nlohmann::json SerializeEntityComponents(uint32_t entityId) const {
			nlohmann::json componentsJson;
			for (const auto& [typeId, storagePtr] : componentStorages) {
				const ComponentData* comp = storagePtr->GetComponentDataPtr(entityId);
				if (comp) {
					componentsJson[comp->GetTypeName()] = comp->Serialize();
				}
			}
			return componentsJson;
		}
		/// @brief フレーム終了処理
		void EndFrame() {
			// 削除予定EEntityに紐づくコンポEネントを全て削除
			for (uint32_t id : entitiesToRemove_) {
				for (auto& [typeId, storage] : componentStorages) {
					storage->RemoveComponent(id);
				}
				activeEntityIds_.erase(id);
			}
			entitiesToRemove_.clear();
		}
		/// @brief エンティティマネージャーをリセット
		void ResetEntiry() {
			componentStorages.clear();
			activeEntityIds_.clear();
			nextEntityId_ = 0;
		}
		/// @brief 指定エンティティを即時削除
		void InstantRemoveEntity(uint32_t id) {
			for (auto& [typeId, storage] : componentStorages) {
				storage->RemoveComponent(id);
			}
			activeEntityIds_.erase(id);
		}
		/// @brief 新規エンティティを生成
		uint32_t CreateEntity() {
			uint32_t id = nextEntityId_++;
			activeEntityIds_.insert(id);
			return id;
		}
		/// @brief 指定エンティティを削除予定に登録
		void RemoveEntity(uint32_t id) {
			entitiesToRemove_.push_back(id);
		}
		/// @brief 指定エンティティがアクティブか判定
		bool IsActiveEntity(uint32_t id) const {
			return activeEntityIds_.find(id) != activeEntityIds_.end();
		}
		/// @brief 指定エンティティにコンポーネントを追加
		template <typename T, typename... Args>
		void EmplaceComponent(uint32_t id, Args&&... args) {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) == componentStorages.end()) {
				componentStorages[typeId] = std::make_unique<ComponentStorage<T>>();
			}
			auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages[typeId]);
			storage.AddComponent(id, T(std::forward<Args>(args)...));
		}
		/// @brief 指定エンティティのコンポーネントを取得
		template <typename T>
		T& GetComponent(uint32_t id) const {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
				return storage.GetComponent(id);
			}
			throw std::runtime_error("Component not found");
		}
		/// @brief 指定エンティティのコンポーネントポインタを取得
		template <typename T>
		T* GetComponentPtr(uint32_t id) const {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
				return storage.GetComponentPtr(id);
			}
			assert(false && "Component strage not found");
			return nullptr;
		}
		/// @brief 指定エンティティのコンポーネントを削除
		template <typename T>
		void RemoveComponent(uint32_t id) {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages[typeId]);
				storage.RemoveComponent(id);
			}
		}
		/// @brief 指定コンポーネントストレージを取得
		template <typename T>
		ComponentStorage<T>& GetComponentStrage() const {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				return static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
			}
			throw std::runtime_error("Component strage not found");
		}
		/// @brief 指定コンポーネントストレージの存在を確認
		template <typename T>
		bool HasComponentStrage() const {
			size_t typeId = typeid(T).hash_code();
			return componentStorages.find(typeId) != componentStorages.end();
		}
		/// @brief 指定エンティティが指定コンポーネントを持っているか判定
		template <typename T>
		bool HasComponent(uint32_t id) const {
			if (id >= nextEntityId_) {
				return false;
			}

			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				const auto& storage = static_cast<const ComponentStorage<T>&>(*componentStorages.at(typeId));
				return storage.HasComponent(id);
			}
			return false;
		}
		/// @brief 次回生成エンティティIDを取得
		uint32_t GetNextEntityId() const {
			return nextEntityId_;
		}
		/// @brief アクティブなエンティティID群を取得
		std::vector<uint32_t> GetActiveEntityIds() const {
			std::vector<uint32_t> sortedIds(activeEntityIds_.begin(), activeEntityIds_.end());
			std::sort(sortedIds.begin(), sortedIds.end());
			return sortedIds;
		}
	};

}
