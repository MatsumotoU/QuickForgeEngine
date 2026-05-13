#pragma once
#include "IDynamicArray.h"
#include <vector>
#include <assert.h>
#include <stdexcept>

#include "engine/include/core/EngineDefines.h"

namespace QFE {
	/// @brief メモリの連続性とアドレスの不変性を保証するstd::vectorをラップしたクラス
	template<typename T>
	class SafeVector : public IDynamicArray<T> {
	public:
		/// @brief コンストラクタ
		explicit SafeVector(size_t size = 512) {
			// std::vectorで確保できるサイズを超えている場合は例外を投げる
			if (size > data_.max_size()) {
				QFE_REPORT_SYSTEM_ERROR("Requested size exceeds maximum capacity of SafeVector.", SystemError::Abort);
			}

			data_.clear();
			data_.reserve(size);
			QFE_LOG(std::string("SafeVector initialized with reserved size: ") + std::to_string(size));
		}

		/// @brief デストラクタ
		virtual ~SafeVector() = default;
		/// @brief 配列の全ての要素を削除して、配列を空にする
		virtual void clear() override {
			data_.clear();
		}

		/// @brief 要素を配列の末尾に追加する（コピーまたはムーブ）
		virtual void push_back(T value) override {
			if (data_.size() >= data_.capacity()) {
				QFE_REPORT_SYSTEM_ERROR("SafeVector capacity exceeded. Consider increasing the reserved size.", SystemError::Abort);
			}
			data_.push_back(std::move(value));
		}

		/// @brief 配列の末尾の要素を削除する
		virtual void pop_back() override {
			if (!data_.empty()) {
				data_.pop_back();
			}
		}

		/// @brief あるインデックスの要素を削除する
		virtual void erase(size_t index) override {
			CheckIndex(index);
			data_.erase(data_.begin() + index);
		}

		virtual T& front() override {
			if (data_.empty()) {
				QFE_REPORT_SYSTEM_ERROR("SafeVector is empty. No elements to access.", SystemError::Abort);
			}
			return data_.front();
		}
		virtual const T& front() const override {
			if (data_.empty()) {
				QFE_REPORT_SYSTEM_ERROR("SafeVector is empty. No elements to access.", SystemError::Abort);
			}
			return data_.front();
		}
		virtual T& back() override {
			if (data_.empty()) {
				QFE_REPORT_SYSTEM_ERROR("SafeVector is empty. No elements to access.", SystemError::Abort);
			}
			return data_.back();
		}
		virtual const T& back() const override {
			if (data_.empty()) {
				QFE_REPORT_SYSTEM_ERROR("SafeVector is empty. No elements to access.", SystemError::Abort);
			}
			return data_.back();
		}

		virtual T* begin() override { return data_.data(); }
		virtual const T* begin() const override { return data_.data(); }
		virtual T* end() override { return data_.data() + data_.size();}
		virtual const T* end() const override {return data_.data() + data_.size();}

		virtual bool empty() const override { return data_.empty(); }
		virtual size_t size() const override { return data_.size(); }
		virtual size_t capacity() const override { return data_.capacity(); }

		/// @brief あるインデックスの要素にアクセスする[]であっても範囲チェックを行う
		virtual T& operator[](size_t index) override {
			CheckIndex(index);
			return data_[index];
		}
		/// @brief あるインデックスの要素にアクセスする[]であっても範囲チェックを行う（const版）
		virtual const T& operator[](size_t index) const override {
			CheckIndex(index);
			return data_[index];
		}

		/// @brief あるインデックスの要素にアクセスする、標準ライブラリの範囲チェックも行うat()を使用
		virtual T& at(size_t index) override {
			CheckIndex(index);
			return data_.at(index);
		}
		/// @brief あるインデックスの要素にアクセスする、標準ライブラリの範囲チェックも行うat()を使用（const版）
		virtual const T& at(size_t index) const override {
			CheckIndex(index);
			return data_.at(index);
		}

	private:
		// @brief インデックスが範囲内かどうかをチェックする関数
		void CheckIndex(size_t index) const {
			// インデックスが範囲内かどうかをチェック
			if (index >= data_.size()) {
				QFE_REPORT_SYSTEM_ERROR(
					"Index out of bounds in SafeVector. Index: " +std::to_string(index) +
					", Size: " + std::to_string(data_.size()), SystemError::Abort);
			}
		}
		std::vector<T> data_;
	};
}