#pragma once
#ifndef Matrix2x2_H
#define Matrix2x2_H

namespace QFE {

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
		/// こE行Eの転地行Eを求めめE
		/// </summary>
		/// <returns>こE行Eの転地行E</returns>
		[[nodiscard]] Matrix2x2 Transpose() const;

	public:

		/// <summary>
		/// 2x2の転置行Eを求めめE
		/// </summary>
		/// <param name="m">2x2の行E</param>
		/// <returns>2x2の転置行E</returns>
		[[nodiscard]] static Matrix2x2 Transpose(const Matrix2x2& m);

	public:
		/// <summary>
		/// 2つの2x2行Eの加算結果を求めめE
		/// </summary>
		/// <param name="m1">1つ目の2x2行E</param>
		/// <param name="m2">2つ目の2x2行E</param>
		/// <returns>2つの2x2行Eの加算結果</returns>
		[[nodiscard]] static Matrix2x2 Add(const Matrix2x2& m1, const Matrix2x2& m2);

		/// <summary>
		/// 2つの2x2行Eの減算結果を求めめE
		/// </summary>
		/// <param name="m1">1つ目の2x2行E</param>
		/// <param name="m2">2つ目の2x2行E</param>
		/// <returns>2つの2x2行Eの減算結果</returns>
		[[nodiscard]] static Matrix2x2 Subtract(const Matrix2x2& m1, const Matrix2x2& m2);

	public:
		/// <summary>
		/// 2x2の単位行Eを求めめE
		/// </summary>
		/// <returns>2x2の単位行E</returns>
		[[nodiscard]] static Matrix2x2 MakeIdentity2x2();
	};

}

#endif
