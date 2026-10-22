#pragma once
#include "SafeVector.h"
#include <cstdint>
#include <vector>
#include <functional>
#include <utility>
#include <memory>
#include <unordered_map>

namespace QFE {
	/// @brief スパースセットコンテナ
	template<typename T>
	class SparseSet {
	public:
		SparseSet() = default;

		/// @brief キーと値のペアを追加する。キーは自動的に割り当てられる。
		uint32_t push_back(const T& value) {
			// 未使用のキーを探す
			uint32_t key = 0;
			while (sparse_.contains(key)) {
				key++;
			}

			dense_keys_.push_back(key);
			dense_.push_back(value);
			sparse_[key] = static_cast<uint32_t>(dense_.size() - 1);
			return key;
		}
		/// @brief キーと値のペアを追加する。キーは自動的に割り当てられる。
		uint32_t push_back(T&& value) {
			// 未使用のキーを探す
			uint32_t key = 0;
			while (sparse_.contains(key)) {
				key++;
			}
			dense_keys_.push_back(key);
			dense_.push_back(std::move(value));
			sparse_[key] = static_cast<uint32_t>(dense_.size() - 1);
			return key;
		}

		/// @brief キーと値のペアを追加または更新する。キーが存在しない場合は新しいペアを追加し、存在する場合は値を更新する。
		void Insert(uint32_t key, const T& value) {
			auto sparseIt = sparse_.find(key);
			if (sparseIt == sparse_.end()) {
				dense_keys_.push_back(key);
				dense_.push_back(value);
				sparse_[key] = static_cast<uint32_t>(dense_.size() - 1);
			} else {
				dense_[sparseIt->second] = value;
			}
		}

		/// @brief キーと値のペアを追加または更新する。キーが存在しない場合は新しいペアを追加し、存在する場合は値を更新する。
		void Insert(uint32_t key, T&& value) {
			auto sparseIt = sparse_.find(key);
			if (sparseIt == sparse_.end()) {
				dense_keys_.push_back(key);
				dense_.push_back(std::move(value));
				sparse_[key] = static_cast<uint32_t>(dense_.size() - 1);
			} else {
				dense_[sparseIt->second] = std::move(value);
			}
		}

		/// @brief キーに対応する値を削除する。キーが存在しない場合は何もしない。
		void Remove(uint32_t key) {
			auto sparseIt = sparse_.find(key);
			if (sparseIt != sparse_.end()) {
				uint32_t index = sparseIt->second;
				
				// O(1)削除：末尾の要素を削除対象の位置に移動する
				dense_[index] = dense_.back();
				dense_keys_[index] = dense_keys_.back();
				
				// 移動してきた要素の元のIDを引いて、sparse_の指す先を更新
				uint32_t moved_key = dense_keys_[index];
				sparse_[moved_key] = index;

				// 末尾を削除
				dense_.pop_back();
				dense_keys_.pop_back();
				sparse_.erase(sparseIt);
			}
		}

		/// @brief キーに対応する値へのポインタを取得する。キーが存在しない場合はnullptrを返す。
		T* Get(uint32_t key) {
			auto sparseIt = sparse_.find(key);
			if (sparseIt != sparse_.end()) {
				return &dense_[sparseIt->second];
			}
			return nullptr;
		}
		const T* Get(uint32_t key) const {
			auto sparseIt = sparse_.find(key);
			if (sparseIt != sparse_.end()) {
				return &dense_[sparseIt->second];
			}
			return nullptr;
		}

		/// @brief キーに対応する要素が存在するかどうかを確認する。
		bool Contains(uint32_t key) const {
			return sparse_.contains(key);
		}

		/// @brief キーの一覧を取得する。
		std::vector<uint32_t> Keys() const {
			// dense_keys_ を使うことで、重いループ検索を使わずに即座にキー一覧を生成可能
			return std::vector<uint32_t>(dense_keys_.begin(), dense_keys_.end());
		}

		/// @brief 値の一覧を取得する。
		std::vector<T> Values() const {
			return std::vector<T>(dense_.begin(), dense_.end());
		}

		/// @brief 全ての有効なコンポーネントに対して関数を実行する。
		/// @param func (uint32_t id, T& component) を引数に取る関数オブジェクトまたはラムダ式
		void Each(const std::function<void(uint32_t, T&)>& func) {
			for (size_t i = 0; i < dense_.size(); ++i) {
				func(dense_keys_[i], dense_[i]);
			}
		}

		/// @brief 読み取り専用のEach
		/// @param func (uint32_t id, const T& component) を引数に取る関数オブジェクトまたはラムダ式
		void Each(const std::function<void(uint32_t, const T&)>& func) const {
			for (size_t i = 0; i < dense_.size(); ++i) {
				func(dense_keys_[i], dense_[i]);
			}
		}

		void clear() {
			sparse_.clear();
			dense_keys_.clear();
			dense_.clear();
		}
		auto begin() { return dense_.begin(); }
		auto end() { return dense_.end(); }
		auto begin() const { return dense_.begin(); }
		auto end() const { return dense_.end(); }
		size_t size() const { return dense_.size(); }
		bool empty() const { return dense_.empty(); }
		void erase(uint32_t key) { Remove(key); }
		T* find(uint32_t key) { return Get(key); }
		const T* find(uint32_t key) const { return Get(key); }
		T& operator[](uint32_t key) {
			if (!Contains(key)) {
				Insert(key, T{});
			}
			return dense_[sparse_.at(key)];
		}
		T& at(uint32_t key) {
			return dense_.at(sparse_.at(key));
		}
		const T& at(uint32_t key) const {
			return dense_.at(sparse_.at(key));
		}
	private:
		std::unordered_map<uint32_t, uint32_t> sparse_;
		SafeVector<uint32_t> dense_keys_;
		SafeVector<T> dense_;
	};
} // namespace QFE
