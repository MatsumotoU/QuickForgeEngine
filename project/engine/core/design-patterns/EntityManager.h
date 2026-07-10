#pragma once
#include "component/ComponentStorage.h"
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>
#include "component/JsonArchive.h"
#include "component/ComponentAutoRegistry.h"
#include "IEntityManager.h"

namespace QFE {
	class EntityManager final: public IEntityManager {
	private:
		std::unordered_map<size_t, std::unique_ptr<IComponentStorage>> componentStorages;
		uint32_t nextEntityId_;
		std::unordered_set<uint32_t> activeEntityIds_;
		std::vector<uint32_t> entitiesToRemove_;

	public:
		~EntityManager() = default;

		/// @brief コンストラクタ。コンポーネントの自動登録を行う。
		EntityManager();
		/// @brief フレームの終了時に、削除予定のエンティティを削除する。
		void EndFrame();

		/// @brief エンティティのコンポーネントをJSONにシリアライズする。
		nlohmann::json SerializeEntityComponents(uint32_t entityId) const;
		/// @brief エンティティのコンポーネントをJSONから復元する。
		void DeserializeEntityComponents(uint32_t entityId, const nlohmann::json& componentsJson);

		/// @brief エンティティIDから、コンポーネント型名の一覧を取得する。
		std::vector<std::string> GetComponentTypeNames(uint32_t entityId) const;

		/// @brief エンティティIDとコンポーネント型名から、コンポーネントの生ポインタを取得する。
		void* GetComponentRaw(uint32_t entityId, const char* componentTypeName) override;
		/// @brief エンティティIDとコンポーネント型名から、コンポーネントを削除する。
		void RemoveComponent(uint32_t entityId, const char* componentTypeName) override;

		/// @brief エンティティマネージャをリセットする。全てのエンティティとコンポーネントを削除する。
		void ResetEntity();
		/// @brief エンティティを即座に削除する。フレーム終了時の削除予定リストには追加されない。
		void InstantRemoveEntity(uint32_t id);
		/// @brief 新しいエンティティを作成する。新しいエンティティIDを返す。
		uint32_t CreateEntity() override;
		/// @brief エンティティを削除予定リストに追加する。フレーム終了時に削除される。
		void RemoveEntity(uint32_t id) override;
		/// @brief エンティティが有効かどうかを判定する。
		bool IsActiveEntity(uint32_t id) const;

		/// @brief 次に生成されるエンティティIDを取得する。
		uint32_t GetNextEntityId() const;
		/// @brief 有効なエンティティIDの一覧を取得する。
		std::vector<uint32_t> GetActiveEntityIds() const;

		/// @brief エンティティにデフォルトコンポーネントを追加する。既に存在する場合は上書きする。
		template <typename T>
		T& AddDefaultComponent(uint32_t id) {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages[typeId]);
				storage.AddDefaultComponent(id);
				return storage.GetComponent(id);
			}
		}
		/// @brief エンティティにコンポーネントが存在するかを判定する。
		template <typename T>
		bool HasComponentStrage() const {
			size_t typeId = typeid(T).hash_code();
			return componentStorages.find(typeId) != componentStorages.end();
		}
		/// @brief エンティティにコンポーネントが存在するかを判定する。
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
		/// @brief エンティティにコンポーネントを追加する。既に存在する場合は上書きする。
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
		/// @brief エンティティIDとコンポーネント型名から、コンポーネントの参照を取得する。
		template <typename T>
		T& GetComponent(uint32_t id) const {
			size_t typeId = typeid(T).hash_code();
			if (HasComponent<T>(id)) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
				return storage.GetComponent(id);
			}
			throw std::runtime_error("Component not found");
		}
		/// @brief エンティティIDとコンポーネント型名から、コンポーネントのポインタを取得する。
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
		/// @brief エンティティIDとコンポーネント型名から、コンポーネントを削除する。
		template <typename T>
		void RemoveComponent(uint32_t id) {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				auto& storage = static_cast<ComponentStorage<T>&>(*componentStorages[typeId]);
				storage.RemoveComponent(id);
			}
		}
		/// @brief コンポーネントストレージを取得する。
		template <typename T>
		ComponentStorage<T>& GetComponentStrage() const {
			size_t typeId = typeid(T).hash_code();
			if (componentStorages.find(typeId) != componentStorages.end()) {
				return static_cast<ComponentStorage<T>&>(*componentStorages.at(typeId));
			}
			assert(false && "Component strage not found");
			return *static_cast<ComponentStorage<T>*>(nullptr);
		}
		/// @brief コンポーネントストレージを取得する。
		template <typename T>
		void Each(const std::function<void(uint32_t, T&)>& func) {
			if (HasComponentStrage<T>()) {
				auto& storage = GetComponentStrage<T>();
				storage.Each(func);
			}
		}
		/// @brief コンポーネントストレージを取得する。
		template <typename T>
		void Each(const std::function<void(uint32_t, const T&)>& func) const {
			if (HasComponentStrage<T>()) {
				const auto& storage = GetComponentStrage<T>();
				storage.Each(func);
			}
		}
	};
}