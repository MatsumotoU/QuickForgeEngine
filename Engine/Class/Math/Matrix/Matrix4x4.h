#pragma once

// 前方宣言
class Vector3;

class Matrix4x4 final{
public:
	float m[4][4];

public:
	Matrix4x4 operator+(const Matrix4x4& other) const;
	Matrix4x4 operator-(const Matrix4x4& other) const;
	Matrix4x4 operator*(const Matrix4x4& other) const;

public:
	/// <summary>
	/// この行列の逆行列を求める
	/// </summary>
	/// <returns>この行列の逆行列</returns>
	[[nodiscard]] Matrix4x4 Inverse() const;

	/// <summary>
	/// この行列の転地行列を求める
	/// </summary>
	/// <returns>この行列の転地行列</returns>
	[[nodiscard]] Matrix4x4 Transpose() const;

public:
	/// <summary>
	/// 4x4の逆行列を求める
	/// </summary>
	/// <param name="m">4x4の行列</param>
	/// <returns>4x4の逆行列</returns>
	[[nodiscard]] static Matrix4x4 Inverse(const Matrix4x4& m);

	/// <summary>
	/// 4x4の転置行列を求める
	/// </summary>
	/// <param name="m">4x4の行列</param>
	/// <returns>4x4の転置行列</returns>
	[[nodiscard]] static Matrix4x4 Transpose(const Matrix4x4& m);

public:
	/// <summary>
	/// 2つの4x4行列の加算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の4x4行列</param>
	/// <param name="m2">2つ目の4x4行列</param>
	/// <returns>2つの4x4行列の加算結果</returns>
	[[nodiscard]] static Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

	/// <summary>
	/// 2つの4x4行列の減算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の4x4行列</param>
	/// <param name="m2">2つ目の4x4行列</param>
	/// <returns>2つの4x4行列の減算結果</returns>
	[[nodiscard]] static Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

	/// <summary>
	/// 2つの4x4行列の乗算結果を求める
	/// </summary>
	/// <param name="m1">1つ目の4x4行列</param>
	/// <param name="m2">2つ目の4x4行列</param>
	/// <returns>2つの4x4行列の乗算結果</returns>
	[[nodiscard]] static Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

public:
	/// <summary>
	/// 4x4の単位行列を求める
	/// </summary>
	/// <returns>4x4の単位行列</returns>
	[[nodiscard]] static Matrix4x4 MakeIndentity4x4();

public:
	/// <summary>
	/// 拡大縮小行列を作成する
	/// </summary>
	/// <param name="scale">拡大縮小の情報</param>
	/// <returns>拡大縮小行列</returns>
	[[nodiscard]] static Matrix4x4 MakeScaleMatrix(const Vector3& translate);

	/// <summary>
	/// 移動行列を作成する
	/// </summary>
	/// <param name="translate">移動量の情報</param>
	/// <returns>移動行列</returns>
	[[nodiscard]] static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	/// <summary>
	/// X軸の回転行列を作成する
	/// </summary>
	/// <param name="radian">ラジアン</param>
	/// <returns>X軸の回転行列</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateXMatrix(const float& radian);

	/// <summary>
	/// Y軸の回転行列を作成する
	/// </summary>
	/// <param name="radian">ラジアン</param>
	/// <returns>X軸の回転行列</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateYMatrix(const float& radian);

	/// <summary>
	/// Z軸の回転行列を作成する
	/// </summary>
	/// <param name="radian">ラジアン</param>
	/// <returns>X軸の回転行列</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateZMatrix(const float& radian);

	/// <summary>
	/// 3次元の回転行列を作成する
	/// </summary>
	/// <param name="radian">ラジアンで構成された回転情報</param>
	/// <returns>3次元の回転行列</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateXYZMatrix(const Vector3& rotate);

	/// <summary>
	/// 3次元のアフィン行列を作成する
	/// </summary>
	/// <param name="scale">拡縮の情報</param>
	/// <param name="rotate">ラジアン</param>
	/// <param name="translate">移動の情報</param>
	/// <returns>3次元のアフィン行列</returns>
	[[nodiscard]] static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

public:
	/// <summary>
	/// 透視投影行列を作成する
	/// </summary>
	/// <param name="fovY">画角</param>
	/// <param name="aspectRatio">アスペクト比</param>
	/// <param name="nearClip">近平面への距離</param>
	/// <param name="farClip">遠平面への距離</param>
	/// <returns>透視投影行列</returns>
	[[nodiscard]] static Matrix4x4 MakePerspectiveFovMatrix(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip);

	/// <summary>
	/// 正規化デバイス行列を作成する
	/// </summary>
	/// <param name="left">画面の左端</param>
	/// <param name="top">画面の上端</param>
	/// <param name="right">画面右端</param>
	/// <param name="bottom">画面の下端</param>
	/// <param name="nearClip">近平面への距離</param>
	/// <param name="farClip">遠平面への距離</param>
	/// <returns>正規化デバイス行列</returns>
	[[nodiscard]] static Matrix4x4 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip);
	
	/// <summary>
	/// ビューポート行列を作成する
	/// </summary>
	/// <param name="left">画面の左端</param>
	/// <param name="top">画面の上端</param>
	/// <param name="width">画面の横幅</param>
	/// <param name="heigh">画面の縦幅</param>
	/// <param name="minDepth">最小深度値</param>
	/// <param name="maxDepth">最大深度値</param>
	/// <returns>ビューポート行列</returns>
	[[nodiscard]] static Matrix4x4 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& heigh, const float& minDepth, const float& maxDepth);
};