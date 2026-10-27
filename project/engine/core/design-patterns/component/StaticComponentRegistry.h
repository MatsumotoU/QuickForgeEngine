#pragma once

#include <assert.h>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/design-patterns/component/ComponentDescriptor.h"
#include "core/design-patterns/component/ComponentStorage.h"

namespace QFE {
	/// @brief コンポーネントの静的レジストリクラス
	class StaticComponentRegistry {
	public:
		static constexpr uint32_t InvalidStorageIndex = (std::numeric_limits<uint32_t>::max)();

		template <typename T>
		/// @brief コンポーネントを登録する
		bool RegisterComponent(const std::string& key, const std::string& category = "Other") {
			// 登録が凍結されている場合は登録できないようにする
			if (isFrozen_) {
				assert(false && "Component registration is frozen. Cannot register new components.");
				return false;
			}
			// 登録済みのコンポーネントかどうかを確認する
			const std::string reflectedTypeName = T::GetTypeName();
			if (key.empty() || category.empty() || reflectedTypeName.empty() || keyToIndexMap_.contains(key) ||
				reflectedTypeNameToIndexMap_.contains(reflectedTypeName)) {
				assert(false && "Component key, category, or reflected type name is empty, or the component is already registered.");
				return false;
			}
			// 登録するコンポーネントの情報を作成する
			ComponentDescriptor descriptor;
			descriptor.key = key;
			descriptor.category = category;
			descriptor.reflectedTypeName = reflectedTypeName;
			descriptor.storageIndex = static_cast<uint32_t>(componentDescriptors_.size());
			descriptor.createStorageFunc = []() -> std::unique_ptr<IComponentStorage> {
				return std::make_unique<ComponentStorage<T>>();
			};
			keyToIndexMap_.emplace(key, descriptor.storageIndex);
			reflectedTypeNameToIndexMap_.emplace(reflectedTypeName, descriptor.storageIndex);
			componentDescriptors_.push_back(std::move(descriptor));
			return true;
		}

		/// @brief 登録されたコンポーネントのストレージインデックスを取得する
		uint32_t GetComponentStorageIndex(const std::string& key) const {
			const auto it = keyToIndexMap_.find(key);
			return it != keyToIndexMap_.end() ? it->second : InvalidStorageIndex;
		}

		/// @brief C++型のリフレクション名からストレージインデックスを取得する
		template <typename T>
		uint32_t GetComponentStorageIndexForType() const {
			static const std::string reflectedTypeName = T::GetTypeName();
			const auto it = reflectedTypeNameToIndexMap_.find(reflectedTypeName);
			return it != reflectedTypeNameToIndexMap_.end() ? it->second : InvalidStorageIndex;
		}

		/// @brief 登録されたコンポーネントの情報を取得する
		const std::vector<ComponentDescriptor>& GetComponentDescriptors() const {
			return componentDescriptors_;
		}

		/// @brief 登録されたコンポーネントのストレージインデックスをキーから取得する
		/// @param key コンポーネントのキー
		/// @return ストレージインデックス(見つからなかった場合はUINT32_MAX)
		uint32_t GetComponentStorageIndexByKey(const std::string& key) const {
			return GetComponentStorageIndex(key);
		}

		/// @brief 登録を凍結する
		void Freeze() {
			isFrozen_ = true;
		}

		/// @brief 登録内容が確定済みかを返す
		bool IsFrozen() const {
			return isFrozen_;
		}

	private:
		bool isFrozen_ = false;
		std::vector<ComponentDescriptor> componentDescriptors_;
		std::unordered_map<std::string, uint32_t> keyToIndexMap_;
		std::unordered_map<std::string, uint32_t> reflectedTypeNameToIndexMap_;
	};
}
