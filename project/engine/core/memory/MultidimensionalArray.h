#pragma once
#include <vector>
#include <array>
#include <cassert>

namespace QFE {
	/// @brief n次元配列を管理するクラス
	template<typename T, size_t N>
	class MultidimensionalArray {
	public:
		MultidimensionalArray() = default;

		/// @brief 配列のサイズを設定します
		void SetSize(const std::array<size_t, N>& sizes) {
			sizes_ = sizes;
			size_t totalSize = 1;
			for (size_t i = 0; i < N; ++i) {
				totalSize *= sizes[i];
			}
			data_.resize(totalSize);
		}
		/// @brief 多次元インデックスで要素にアクセスします
		T& At(const std::array<uint32_t, N>& indices) {
			// インデックスの型が uint32_t でも size_t に変換して計算できるようにする
			std::array<size_t, N> idx;
			for (size_t i = 0; i < N; ++i) idx[i] = static_cast<size_t>(indices[i]);
			return data_[CalculateFlatIndex(idx)];
		}
		/// @brief 多次元インデックスで要素にアクセスします（const版）
		const T& At(const std::array<uint32_t, N>& indices) const {
			std::array<size_t, N> idx;
			for (size_t i = 0; i < N; ++i) idx[i] = static_cast<size_t>(indices[i]);
			return data_[CalculateFlatIndex(idx)];
		}
		/// @brief 多次元インデックスで要素にアクセスします（size_t版）
		T& At(const std::array<size_t, N>& indices) {
			return data_[CalculateFlatIndex(indices)];
		}
		/// @brief 多次元インデックスで要素にアクセスします（const版）
		const T& At(const std::array<size_t, N>& indices) const {
			return data_[CalculateFlatIndex(indices)];
		}

	private:
		/// @brief 多次元インデックスを1次元のフラットインデックスに変換する
		size_t CalculateFlatIndex(const std::array<size_t, N>& indices) const {
			size_t flatIndex = 0;
			size_t multiplier = 1;

			for (size_t i = N; i-- > 0;) {
				assert(indices[i] < sizes_[i] && "Index out of bounds!");
				flatIndex += indices[i] * multiplier;
				multiplier *= sizes_[i];
			}
			return flatIndex;
		}

	private:
		std::vector<T> data_;
		std::array<size_t, N> sizes_{};
	};
}