#pragma once
#ifndef Matrix2x2_H
#define Matrix2x2_H

namespace QFE::MATH {

	class Matrix2x2 final {
	public:
		float m[2][2];

	public:
		float Get(int row, int col) const {
			return m[row][col];
		}
		void Set(int row, int col, float value) {
			m[row][col] = value;
		}

	public:

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix2x2 Transpose() const;

	public:

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix2x2 Transpose(const Matrix2x2& m);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix2x2 Add(const Matrix2x2& m1, const Matrix2x2& m2);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix2x2 Subtract(const Matrix2x2& m1, const Matrix2x2& m2);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix2x2 MakeIdentity2x2();
	};

}

#endif
