#pragma once

// 前方宣言
class Vector2;

class Matrix3x3 final{
public:
	float m[3][3];

public:
	Matrix3x3 operator+(const Matrix3x3& other);
	Matrix3x3 operator-(const Matrix3x3& other);
	Matrix3x3 operator*(const Matrix3x3& other);

public:
	/// <summary>
	/// この行列の逆行列を求める
	/// </summary>
	/// <returns>この行列の逆行列</returns>
	[[nodiscard]] Matrix3x3 Inverse() const;

	/// <summary>
	/// この行列の転地行列を求める
	/// </summary>
	/// <returns>この行列の転地行列</returns>
	[[nodiscard]] Matrix3x3 Transpose() const;

public:
	/// <summary>
	/// 3x3の逆行列を求める
	/// </summary>
	/// <param name="m">3x3の行列</param>
	/// <returns>3x3の逆行列</returns>
	[[nodiscard]] static Matrix3x3 Inverse(const Matrix3x3& m);

	/// <summary>
	/// 3x3の転置行列を求める
	/// </summary>
	/// <param name="m">3x3の行列</param>
	/// <returns>3x3の転置行列</returns>
	[[nodiscard]] static Matrix3x3 Transpose(const Matrix3x3& m);

public:
	/// <summary>
	/// 2つの3x3行列の加算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の3x3行列</param>
	/// <param name="m2">2つ目の3x3行列</param>
	/// <returns>2つの3x3行列の加算結果</returns>
	[[nodiscard]] static Matrix3x3 Add(const Matrix3x3& m1, const Matrix3x3& m2);

	/// <summary>
	/// 2つの3x3行列の減算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の3x3行列</param>
	/// <param name="m2">2つ目の3x3行列</param>
	/// <returns>2つの3x3行列の減算結果</returns>
	[[nodiscard]] static Matrix3x3 Subtract(const Matrix3x3& m1, const Matrix3x3& m2);

	/// <summary>
	/// 2つの3x3行列の乗算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の3x3行列</param>
	/// <param name="m2">2つ目の3x3行列</param>
	/// <returns>2つの3x3行列の乗算結果</returns>
	[[nodiscard]] static Matrix3x3 Multiply(const Matrix3x3& m1, const Matrix3x3& m2);

public:
	/// <summary>
	/// 3x3の単位行列を求める
	/// </summary>
	/// <returns>3x3の単位行列</returns>
	[[nodiscard]] static Matrix3x3 MakeIdentity3x3();

public:
	/// <summary>
	/// 拡大縮小行列を作成する
	/// </summary>
	/// <param name="scale">拡大縮小の情報</param>
	/// <returns>拡大縮小行列</returns>
	[[nodiscard]] Matrix3x3 MakeScaleMatrix(const Vector2& scale) const;

	/// <summary>
	/// 回転行列を作成する
	/// </summary>
	/// <param name="theta">ラジアン</param>
	/// <returns>回転行列</returns>
	[[nodiscard]] Matrix3x3 MakeRotateMatrix(const float& theta) const;

	/// <summary>
	/// 移動行列を作成する
	/// </summary>
	/// <param name="translate">移動量の情報</param>
	/// <returns>移動行列</returns>
	[[nodiscard]] Matrix3x3 MakeTranslateMatrix(const Vector2& translate) const;

	/// <summary>
	/// アフィン行列を作成する
	/// </summary>
	/// <param name="scale">拡大縮小の情報</param>
	/// <param name="rotate">ラジアン</param>
	/// <param name="translate">移動量の情報</param>
	/// <returns>アフィン行列</returns>
	[[nodiscard]] Matrix3x3 MakeAffineMatrix(const Vector2& scale, const float& rotate, const Vector2& translate) const;

public:
	/// <summary>
	/// 正射影行列を作成する
	/// </summary>
	/// <param name="left">画面の左端座標</param>
	/// <param name="top">画面の上端座標</param>
	/// <param name="right">画面の右端座標</param>
	/// <param name="bottom">画面の下端座標</param>
	/// <returns>正射影行列</returns>
	[[nodiscard]] Matrix3x3 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom) const;

	/// <summary>
	/// ビューポート行列を作成する
	/// </summary>
	/// <param name="left">画面の左端座標</param>
	/// <param name="top">画面の上端座標</param>
	/// <param name="width">画面の右端座標</param>
	/// <param name="height">画面の下端座標</param>
	/// <returns>ビューポート行列</returns>
	[[nodiscard]] Matrix3x3 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height) const;

	/// <summary>
	/// ワールドビュープロジェクション行列を作成する
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="viewMatrix">ビュー行列</param>
	/// <param name="orthoMatrix">正射影行列</param>
	/// <param name="viewportMatrix">ビューポート行列</param>
	/// <returns>ワールドビュープロジェクション行列</returns>
	[[nodiscard]] Matrix3x3 MakeWvpVpMatrix(const Matrix3x3& worldMatrix, const Matrix3x3& viewMatrix, const Matrix3x3& orthoMatrix, const Matrix3x3& viewportMatrix) const;
};