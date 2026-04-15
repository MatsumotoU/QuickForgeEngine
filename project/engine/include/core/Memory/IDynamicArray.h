#pragma once
namespace QFE {
	/// @brief 動的配列のインターフェース	 
	template<typename T>
	class IDynamicArray {
	public:
		/// @brief デストラクタ
		virtual ~IDynamicArray() = default;

		/// @brief 配列の全ての要素を削除して、配列を空にする
		virtual void clear() = 0;
		/// @brief 追加する要素を指定して、配列の末尾に要素を追加する
		virtual void push_back(const T& value) = 0;
		/// @brief 追加する要素を指定して、配列の末尾に要素を追加する（ムーブセマンティクス対応版）
		virtual void emplace_back(T&& value) = 0;
		/// @brief 配列の末尾の要素を削除する
		virtual void pop_back() = 0;
		/// @brief ある要素を指定して削除する
		virtual void erase(const T& value) = 0;
		/// @brief あるインデックスの要素を削除する
		virtual void erase(size_t index) = 0;

		/// @brief 配列の最初のイテレータを取得する
		virtual T* begin() = 0;
		/// @brief 配列の最初のイテレータを取得する（const版）
		virtual const T* begin() const = 0;

		/// @brief 配列の最後のイテレータを取得する
		virtual T* end() = 0;
		/// @brief 配列の最後のイテレータを取得する（const版）
		virtual const T* end() const = 0;

		/// @brief 配列の先頭の要素にアクセスする
		virtual T& front() = 0;
		/// @brief 配列の先頭の要素にアクセスする（const版）
		virtual const T& front() const = 0;
		/// @brief 配列の末尾の要素にアクセスする
		virtual T& back() = 0;
		/// @brief 配列の末尾の要素にアクセスする（const版）
		virtual const T& back() const = 0;

		/// @brief 配列が空かどうかを判定する
		virtual bool empty() const = 0;
		/// @brief 配列の要素数を取得する
		virtual size_t size() const = 0;
		/// @brief 配列の容量を取得する
		virtual size_t capacity() const = 0;

		/// @brief あるインデックスの要素にアクセスする
		virtual T& operator[](size_t index) = 0;
		/// @brief あるインデックスの要素にアクセスする（const版）
		virtual const T& operator[](size_t index) const = 0;

		/// @brief あるインデックスの要素にアクセスする（範囲チェックあり）
		virtual T& at(size_t index) = 0;
		/// @brief あるインデックスの要素にアクセスする（範囲チェックあり、const版）
		virtual const T& at(size_t index) const = 0;
	};
}