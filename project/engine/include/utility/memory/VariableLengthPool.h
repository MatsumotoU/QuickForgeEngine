#pragma once
#include "IVariableLengthPoolContainer.h"
#include <vector>
#include <stack>
namespace QFE {
	/// @brief 可変長オブジェクトプールコンテナ
	template <typename T>
	class VariableLengthPool final : public IVariableLengthPoolContainer {
	public:
		// デフォルトだと10個のオブジェクトをプール
		VariableLengthPool(size_t initialSize = 10) {
			for (size_t i = 0; i < initialSize; ++i) {
				freeIndices_.push(i);
				pool_.emplace_back();
			}
		}

		// * コンテナ操作 * //
		// オブジェクトを追加（インデックス指定）
		uint32_t Add(const T& obj) {
			if (freeIndices_.empty()) {
				size_t newIndex = pool_.size();
				pool_.emplace_back(obj);
				return static_cast<uint32_t>(newIndex);
			} else {
				size_t index = freeIndices_.top();
				freeIndices_.pop();
				pool_[index] = obj;
				return static_cast<uint32_t>(index);
			}
		}
		// オブジェクトを追加（ムーブ指定）
		uint32_t Add(T&& obj) {
			if (freeIndices_.empty()) {
				size_t newIndex = pool_.size();
				pool_.emplace_back(std::move(obj));
				return static_cast<uint32_t>(newIndex);
			} else {
				size_t index = freeIndices_.top();
				freeIndices_.pop();
				pool_[index] = std::move(obj);
				return static_cast<uint32_t>(index);
			}
		}
		// オブジェクトを取得
		T* Acquire() {
			if (freeIndices_.empty()) {
				size_t newIndex = pool_.size();
				pool_.emplace_back();
				return &pool_[newIndex];
			} else {
				size_t index = freeIndices_.top();
				freeIndices_.pop();
				return &pool_[index];
			}
		}
		// オブジェクトを取得（インデックス指定）
		T& Get(uint32_t index) {
			return pool_[index];
		}
		// オブジェクトを解放
		void Release(T* obj) {
			size_t index = obj - &pool_[0];
			freeIndices_.push(index);
		}
		// オブジェクトを解放（インデックス指定）
		void Release(uint32_t index) {
			freeIndices_.push(index);
		}
		// 全オブジェクトを解放
		void ReleaseAll() {
			while (!freeIndices_.empty()) {
				freeIndices_.pop();
			}
			for (size_t i = 0; i < pool_.size(); ++i) {
				freeIndices_.push(i);
			}
		}
		// プールを解放
		void Release() override {
			while (!freeIndices_.empty()) {
				freeIndices_.pop();
			}
			for (size_t i = 0; i < pool_.size(); ++i) {
				freeIndices_.push(i);
			}
		}

		// * コンテナ状態確認 * //
		// プール内のオブジェクト数を取得
		size_t Size() const override {
			return pool_.size();
		}
		// 使用中のオブジェクト数を取得
		size_t UsedSize() const override {
			return pool_.size() - freeIndices_.size();
		}
		// プールが空かどうかを確認
		bool IsEmpty() const override {
			return UsedSize() == 0;
		}
		// プールが満杯かどうかを確認
		bool IsFull() const override {
			return freeIndices_.empty();
		}
		// 空きインデックス数を取得
		size_t FreeSize() const override {
			return freeIndices_.size();
		}
		// 型名を取得
		const char* GetTypeName() const override {
			return typeid(T).name();
		}

	private:
		std::vector<T> pool_;
		std::stack<size_t> freeIndices_;
	};
}