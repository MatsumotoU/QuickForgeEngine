#pragma once
#include <string>
#include <unordered_map>

namespace QFE {
	template<typename T>
	/// @brief 名前とデータを一意に管理するコンテナ
	class UniqueContainer {
	public:
		/// @brief データを追加し、ハンドルを返す
		uint32_t Add(const std::string& name, const T& data) {
			// データが既に存在する場合は、既存のハンドルを返す
			for (const auto& pair : handleMap_) {
				if (pair.second == name) {
					return pair.first;
				}
			}
			// データが存在しない場合は、新しいハンドルを生成して追加
			uint32_t handle = static_cast<uint32_t>(handleMap_.size() + 1);
			handleMap_[handle] = name;
			dataMap_[name] = data;
			return handle;
		}
		/// @brief データをムーブして追加し、ハンドルを返す
		uint32_t Add(const std::string& name, T&& data) {
			// データが既に存在する場合は、既存のハンドルを返す
			for (const auto& pair : handleMap_) {
				if (pair.second == name) {
					return pair.first;
				}
			}
			// データが存在しない場合は、新しいハンドルを生成して追加
			uint32_t handle = static_cast<uint32_t>(handleMap_.size() + 1);
			handleMap_[handle] = name;
			dataMap_[name] = std::move(data);
			return handle;
		}

		/// @brief 名前からハンドルを取得する
		uint32_t GetHandle(const std::string& name) const {
			for (const auto& pair : handleMap_) {
				if (pair.second == name) {
					return pair.first;
				}
			}
			return UINT32_MAX; // Not found
		}
		/// @brief ハンドルからデータを取得する
		const T* GetData(uint32_t handle) const {
			auto it = handleMap_.find(handle);
			if (it != handleMap_.end()) {
				const std::string& name = it->second;
				auto dataIt = dataMap_.find(name);
				if (dataIt != dataMap_.end()) {
					return &dataIt->second;
				}
			}
			return nullptr; // Not found
		}

		/// @brief ある名前のデータが存在するかどうかを確認する
		bool Contains(const std::string& name) const {
			return dataMap_.find(name) != dataMap_.end();
		}
		/// @brief あるハンドルのデータが存在するかどうかを確認する
		bool Contains(uint32_t handle) const {
			return handleMap_.find(handle) != handleMap_.end();
		}

	private:
		std::unordered_map<uint32_t, std::string> handleMap_;
		std::unordered_map<std::string, T> dataMap_;
	};
}