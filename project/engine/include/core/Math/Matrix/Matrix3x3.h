#pragma once
#ifndef Matrix3x3_H
#define Matrix3x3_H

// 前方宣言
class Vector2;

class Matrix3x3 final{
public:
	float m[3][3];

public:
	float Get(int row, int col) const {
		return m[row][col];
	}
	void Set(int row, int col, float value) {
		m[row][col] = value;
	}

public:
	Matrix3x3 operator+(const Matrix3x3& other);
	Matrix3x3 operator-(const Matrix3x3& other);
	Matrix3x3 operator*(const Matrix3x3& other);

public:
	/// <summary>
	/// こ�E行�Eの送E���Eを求めめE
	/// </summary>
	/// <returns>こ�E行�Eの送E���E</returns>
	[[nodiscard]] Matrix3x3 Inverse() const;

	/// <summary>
	/// こ�E行�Eの転地行�Eを求めめE
	/// </summary>
	/// <returns>こ�E行�Eの転地行�E</returns>
	[[nodiscard]] Matrix3x3 Transpose() const;

public:
	/// <summary>
	/// 3x3の送E���Eを求めめE
	/// </summary>
	/// <param name="m">3x3の行�E</param>
	/// <returns>3x3の送E���E</returns>
	[[nodiscard]] static Matrix3x3 Inverse(const Matrix3x3& m);

	/// <summary>
	/// 3x3の転置行�Eを求めめE
	/// </summary>
	/// <param name="m">3x3の行�E</param>
	/// <returns>3x3の転置行�E</returns>
	[[nodiscard]] static Matrix3x3 Transpose(const Matrix3x3& m);

public:
	/// <summary>
	/// 2つの3x3行�Eの加算結果を求めめE
	/// </summary>
	/// <param name="m1">1つ目の3x3行�E</param>
	/// <param name="m2">2つ目の3x3行�E</param>
	/// <returns>2つの3x3行�Eの加算結果</returns>
	[[nodiscard]] static Matrix3x3 Add(const Matrix3x3& m1, const Matrix3x3& m2);

	/// <summary>
	/// 2つの3x3行�Eの減算結果を求めめE
	/// </summary>
	/// <param name="m1">1つ目の3x3行�E</param>
	/// <param name="m2">2つ目の3x3行�E</param>
	/// <returns>2つの3x3行�Eの減算結果</returns>
	[[nodiscard]] static Matrix3x3 Subtract(const Matrix3x3& m1, const Matrix3x3& m2);

	/// <summary>
	/// 2つの3x3行�Eの乗算結果を求めめE
	/// </summary>
	/// <param name="m1">1つ目の3x3行�E</param>
	/// <param name="m2">2つ目の3x3行�E</param>
	/// <returns>2つの3x3行�Eの乗算結果</returns>
	[[nodiscard]] static Matrix3x3 Multiply(const Matrix3x3& m1, const Matrix3x3& m2);

public:
	/// <summary>
	/// 3x3の単位行�Eを求めめE
	/// </summary>
	/// <returns>3x3の単位行�E</returns>
	[[nodiscard]] static Matrix3x3 MakeIdentity3x3();

public:
	/// <summary>
	/// 拡大縮小行�Eを作�Eする
	/// </summary>
	/// <param name="scale">拡大縮小�E惁E��</param>
	/// <returns>拡大縮小行�E</returns>
	[[nodiscard]] Matrix3x3 MakeScaleMatrix(const Vector2& scale) const;

	/// <summary>
	/// 回転行�Eを作�Eする
	/// </summary>
	/// <param name="theta">ラジアン</param>
	/// <returns>回転行�E</returns>
	[[nodiscard]] Matrix3x3 MakeRotateMatrix(const float& theta) const;

	/// <summary>
	/// 移動行�Eを作�Eする
	/// </summary>
	/// <param name="translate">移動量の惁E��</param>
	/// <returns>移動行�E</returns>
	[[nodiscard]] Matrix3x3 MakeTranslateMatrix(const Vector2& translate) const;

	/// <summary>
	/// アフィン行�Eを作�Eする
	/// </summary>
	/// <param name="scale">拡大縮小�E惁E��</param>
	/// <param name="rotate">ラジアン</param>
	/// <param name="translate">移動量の惁E��</param>
	/// <returns>アフィン行�E</returns>
	[[nodiscard]] Matrix3x3 MakeAffineMatrix(const Vector2& scale, const float& rotate, const Vector2& translate) const;

public:
	/// <summary>
	/// 正封E��行�Eを作�Eする
	/// </summary>
	/// <param name="left">画面の左端座樁E/param>
	/// <param name="top">画面の上端座樁E/param>
	/// <param name="right">画面の右端座樁E/param>
	/// <param name="bottom">画面の下端座樁E/param>
	/// <returns>正封E��行�E</returns>
	[[nodiscard]] Matrix3x3 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom) const;

	/// <summary>
	/// ビューポ�Eト行�Eを作�Eする
	/// </summary>
	/// <param name="left">画面の左端座樁E/param>
	/// <param name="top">画面の上端座樁E/param>
	/// <param name="width">画面の右端座樁E/param>
	/// <param name="height">画面の下端座樁E/param>
	/// <returns>ビューポ�Eト行�E</returns>
	[[nodiscard]] Matrix3x3 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height) const;

	/// <summary>
	/// ワールドビュープロジェクション行�Eを作�Eする
	/// </summary>
	/// <param name="worldMatrix">ワールド行�E</param>
	/// <param name="viewMatrix">ビュー行�E</param>
	/// <param name="orthoMatrix">正封E��行�E</param>
	/// <param name="viewportMatrix">ビューポ�Eト行�E</param>
	/// <returns>ワールドビュープロジェクション行�E</returns>
	[[nodiscard]] Matrix3x3 MakeWvpVpMatrix(const Matrix3x3& worldMatrix, const Matrix3x3& viewMatrix, const Matrix3x3& orthoMatrix, const Matrix3x3& viewportMatrix) const;
};

#endif
