#pragma once
#include <vector>

// 前方宣言
class Matrix4x4;

// TODO: 使える座標系を増やす

class Vector3 final {
public:
	float x;
	float y;
	float z;

public:
	Vector3 operator-()const { return { -x, -y, -z }; };

	Vector3 operator+(const Vector3& other) const { return { x + other.x, y + other.y, z + other.z }; }
	Vector3 operator-(const Vector3& other) const { return { x - other.x, y - other.y, z - other.z }; }
	Vector3 operator*(const Vector3& other) const { return { x * other.x, y * other.y, z * other.z }; }
	Vector3 operator/(const Vector3& other) const { return { x / other.x, y / other.y, z / other.z }; }
	Vector3 operator=(const Vector3& other) { return { x = other.x, y = other.y, z = other.z }; }
	Vector3 operator+=(const Vector3& other) { return { x += other.x, y += other.y, z += other.z }; }
	Vector3 operator-=(const Vector3& other) { return { x -= other.x, y -= other.y, z -= other.z }; }
	Vector3 operator*=(const Vector3& other) { return { x *= other.x, y *= other.y, z *= other.z }; }
	Vector3 operator/=(const Vector3& other) { return { x /= other.x, y /= other.y, z /= other.z }; }

	Vector3 operator+(const float& other) const { return { x + other,y + other,z + other }; };
	Vector3 operator-(const float& other) const { return { x - other,y - other,z - other }; };
	Vector3 operator/(const float& other) const { return { x / other,y / other,z / other }; };
	Vector3 operator*(const float& other) const { return { x * other,y * other,z * other }; };

public:
	/// <summary>
	/// 3次元ベクトルの長さを求める
	/// </summary>
	/// <returns>3次元ベクトルの長さ</returns>
	[[nodiscard]] float Length() const;

	/// <summary>
	/// 正規化された3次元ベクトルを求める
	/// </summary>
	/// <returns>正規化された3次元ベクトル</returns>
	[[nodiscard]] Vector3 Normalize() const;

public:
	/// <summary>
	/// 正規化された3次元ベクトルを求める
	/// </summary>
	/// <returns>正規化された3次元ベクトル</returns>
	[[nodiscard]] static Vector3 Normalize(const Vector3& vector);

public:
	/// <summary>
	/// 2つの3次元ベクトルの内積を求める
	/// </summary>
	/// <param name="v1">1つ目の3次元ベクトル</param>
	/// <param name="v2">2つ目の3次元ベクトル</param>
	/// <returns>2つの3次元ベクトルの内積</returns>
	[[nodiscard]] static float Dot(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// 2つの3次元ベクトルの外積を求める
	/// </summary>
	/// <param name="v1">1つ目の3次元ベクトル</param>
	/// <param name="v2">2つ目の3次元ベクトル</param>
	/// <returns>2つの3次元ベクトルの外積</returns>
	[[nodiscard]] static Vector3 Cross(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// 線形補完を求める
	/// </summary>
	/// <param name="v1">1つ目の3次元ベクトル(t=0)</param>
	/// <param name="v2">2つ目の3次元ベクトル(t=1)</param>
	/// <param name="t">補間に使用する値(0~1)</param>
	/// <returns>線形補完</returns>
	[[nodiscard]] static Vector3 Lerp(const Vector3& v1, const Vector3& v2,float t);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="v1"></param>
	/// <param name="v2"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	[[nodiscard]] static Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);

	/// <summary>
	/// ベジェ曲線を求める
	/// </summary>
	/// <param name="p0">始点</param>
	/// <param name="p1">制御点</param>
	/// <param name="p2">終点</param>
	/// <param name="t">媒介変数</param>
	/// <returns>ベジェ曲線</returns>
	[[nodiscard]] static Vector3 BezierCurve(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t);

	/// <summary>
	/// スプライン曲線を求める
	/// </summary>
	/// <param name="controlPoints"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	[[nodiscard]] static Vector3 CatmullRom(const std::vector<Vector3>& controlPoints, float t);

public:
	/// <summary>
	/// ワールド座標に変換する
	/// </summary>
	/// <param name="vector">変換元ローカル座標</param>
	/// <param name="matrix">変換に使用する行列</param>
	/// <returns>ワールド座標</returns>
	[[nodiscard]] static Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

public:
	/// <summary>
	/// ある点からある点を見るrotationを返します(ラジアン)
	/// </summary>
	/// <param name="eyePosition">見る地点</param>
	/// <param name="targetPosition">見る対象</param>
	/// <returns>ラジアン</returns>
	[[nodiscard]] static Vector3 LookAt(const Vector3& eyePosition, const Vector3& targetPosition);

	/// <summary>
	/// 正射影ベクトルを求める
	/// </summary>
	/// <param name="v1">1つ目の3次元ベクトル</param>
	/// <param name="v2">2つ目の3次元ベクトル</param>
	/// <returns>正射影ベクトル</returns>
	[[nodiscard]] static Vector3 Project(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// あるベクトルの垂直なベクトルを求める
	/// </summary>
	/// <param name="vector">あるベクトル</param>
	/// <returns>あるベクトルの垂直なベクトル</returns>
	[[nodiscard]] static Vector3 Perpendicular(const Vector3& vector);

public:
	/// <summary>
	/// 球面座標系から直交座標系に変換する
	/// </summary>
	/// <param name="rtp">x=半径,y=シータ,z=ファイ</param>
	/// <returns></returns>
	[[nodiscard]] static Vector3 SphericalToCartesian(const Vector3& rtp);
	/// <summary>
	/// デカルト座標系から球面座標系に変換する
	/// </summary>
	/// <param name="xyz"></param>
	/// <returns></returns>
	[[nodiscard]] static Vector3 CartesianToSpherical(const Vector3& xyz);

};







