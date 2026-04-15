#pragma once
namespace QFE {
	// * 固定長ベクタ * //
	template <typename T, size_t N>
	class IStaticVector {
	public:
		virtual ~IStaticVector() = default;
		/// @brief ベクタのサイズを取得
		virtual size_t size() const = 0;
		/// @brief ベクタの容量を取得
		virtual size_t capacity() const = 0;
		/// @brief ベクタが空かどうかを確認
		virtual bool is_empty() const = 0;
		/// @brief ベクタが満杯かどうかを確認
		virtual bool is_full() const = 0;
		/// @brief ベクタに要素を追加
		virtual void push_back(const T& value) = 0;
		/// @brief ベクタから要素を削除
		virtual void pop_back() = 0;
		/// @brief ベクタの要素にアクセス
		virtual T& at(size_t index) = 0;
		virtual const T& at(size_t index) const = 0;
	};
}