#pragma once
class Matrix2x2 final{
public:
	float m[2][2];

public:

	/// <summary>
	/// この行列の転地行列を求める
	/// </summary>
	/// <returns>この行列の転地行列</returns>
	[[nodiscard]] Matrix2x2 Transpose() const;

public:

	/// <summary>
	/// 2x2の転置行列を求める
	/// </summary>
	/// <param name="m">2x2の行列</param>
	/// <returns>2x2の転置行列</returns>
	[[nodiscard]] static Matrix2x2 Transpose(const Matrix2x2& m);

public:
	/// <summary>
	/// 2つの2x2行列の加算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の2x2行列</param>
	/// <param name="m2">2つ目の2x2行列</param>
	/// <returns>2つの2x2行列の加算結果</returns>
	[[nodiscard]] static Matrix2x2 Add(const Matrix2x2& m1, const Matrix2x2& m2);

	/// <summary>
	/// 2つの2x2行列の減算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の2x2行列</param>
	/// <param name="m2">2つ目の2x2行列</param>
	/// <returns>2つの2x2行列の減算結果</returns>
	[[nodiscard]] static Matrix2x2 Subtract(const Matrix2x2& m1, const Matrix2x2& m2);

public:
	/// <summary>
	/// 2x2の単位行列を求める
	/// </summary>
	/// <returns>2x2の単位行列</returns>
	[[nodiscard]] static Matrix2x2 MakeIdentity2x2();
};