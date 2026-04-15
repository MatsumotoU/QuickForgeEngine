#pragma once
#include "IDynamicArray.h"
#include <vector>
#include <assert.h>

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

namespace QFE {
	/// @brief メモリの連続性とアドレスの不変性を保証するstd::vectorをラップしたクラス
	template<typename T>
	class SafeVector : public IDynamicArray<T> {
	public:
		/// @brief コンストラクタ
		explicit SafeVector(size_t size = 50) {
			data_.clear();
			data_.reserve(size);
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::string("SafeVector initialized with reserved size: ") + std::to_string(size));
#endif // QFE_OPTIMIZE_OFF
		}

		/// @brief デストラクタ
		virtual ~SafeVector() = default;
		/// @brief 配列の全ての要素を削除して、配列を空にする
		virtual void clear() override {
			data_.clear();
		}

		/// @brief 追加する要素を指定して、配列の末尾に要素を追加する
		virtual void push_back(const T& value) override {
			assert(data_.size() < data_.capacity() && "SafeVector capacity exceeded. Consider increasing the reserved size.");
			data_.push_back(value);
		}

		/// @brief 追加する要素を指定して、配列の末尾に要素を追加する（ムーブセマンティクス対応版）
		virtual void emplace_back(T&& value) override {
			assert(data_.size() < data_.capacity() && "SafeVector capacity exceeded. Consider increasing the reserved size.");
			data_.emplace_back(std::move(value));
		}

		/// @brief 配列の末尾の要素を削除する
		virtual void pop_back() override {
			if (!data_.empty()) {
				data_.pop_back();
			}
		}

		/// @brief ある要素を指定して削除する
		virtual void erase(const T& value) override {
			auto it = std::find(data_.begin(), data_.end(), value);
			if (it != data_.end()) {
				data_.erase(it);
			}
		}

		/// @brief あるインデックスの要素を削除する
		virtual void erase(size_t index) override {
			if (index < data_.size()) {
				data_.erase(data_.begin() + index);
			}
		}

		virtual T* begin() override { return data_.data(); }
		virtual const T* begin() const override { return data_.data(); }
		virtual T& front() override { return data_.front(); }
		virtual const T& front() const override { return data_.front(); }
		virtual T& back() override { return data_.back(); }
		virtual const T& back() const override { return data_.back(); }
		virtual T* end() override { return data_.data() + data_.size(); }
		virtual const T* end() const override { return data_.data() + data_.size(); }

		virtual bool empty() const override { return data_.empty(); }
		virtual size_t size() const override { return data_.size(); }
		virtual size_t capacity() const override { return data_.capacity(); }

		/// @brief あるインデックスの要素にアクセスする[]であっても範囲チェックを行う
		virtual T& operator[](size_t index) override {
			assert(index < data_.size() && "Index out of bounds in SafeVector.");
			return data_[index];
		}
		/// @brief あるインデックスの要素にアクセスする[]であっても範囲チェックを行う（const版）
		virtual const T& operator[](size_t index) const override {
			assert(index < data_.size() && "Index out of bounds in SafeVector.");
			return data_[index];
		}

		/// @brief あるインデックスの要素にアクセスする、標準ライブラリの範囲チェックも行うat()を使用
		virtual T& at(size_t index) override {
			assert(index < data_.size() && "Index out of bounds in SafeVector.");
			return data_.at(index);
		}
		/// @brief あるインデックスの要素にアクセスする、標準ライブラリの範囲チェックも行うat()を使用（const版）
		virtual const T& at(size_t index) const override {
			assert(index < data_.size() && "Index out of bounds in SafeVector.");
			return data_.at(index);
		}

	private:
		std::vector<T> data_;
	};
}