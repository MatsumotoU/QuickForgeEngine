#pragma once
#ifndef VECTOR3_H
#define VECTOR3_H

#include <vector>

// 前方宣言
class Matrix4x4;
class Camera;
class Vector2;

// TODO: 使える座標系を増やぁE

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
	[[nodiscard]] static Vector3 Zero();

	[[nodiscard]] Vector2 XY();

public:
	/// <summary>
	/// 3次允E�Eクトルの長さを求めめE
	/// </summary>
	/// <returns>3次允E�Eクトルの長ぁE/returns>
	[[nodiscard]] float Length() const;

	/// <summary>
	/// ベクトルの長さ�E二乗を求めめE
	/// </summary>
	/// <returns></returns>
	[[nodiscard]] float LengthSq() const;

	/// <summary>
	/// 正規化されぁE次允E�Eクトルを求めめE
	/// </summary>
	/// <returns>正規化されぁE次允E�Eクトル</returns>
	[[nodiscard]] Vector3 Normalize() const;

public:
	/// <summary>
	/// 正規化されぁE次允E�Eクトルを求めめE
	/// </summary>
	/// <returns>正規化されぁE次允E�Eクトル</returns>
	[[nodiscard]] static Vector3 Normalize(const Vector3& vector);

public:
	/// <summary>
	/// 2つの3次允E�Eクトルの冁E��を求めめE
	/// </summary>
	/// <param name="v1">1つ目の3次允E�Eクトル</param>
	/// <param name="v2">2つ目の3次允E�Eクトル</param>
	/// <returns>2つの3次允E�Eクトルの冁E��E/returns>
	[[nodiscard]] static float Dot(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// 2つの3次允E�Eクトルの外積を求めめE
	/// </summary>
	/// <param name="v1">1つ目の3次允E�Eクトル</param>
	/// <param name="v2">2つ目の3次允E�Eクトル</param>
	/// <returns>2つの3次允E�Eクトルの外穁E/returns>
	[[nodiscard]] static Vector3 Cross(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// 線形補完を求めめE
	/// </summary>
	/// <param name="v1">1つ目の3次允E�Eクトル(t=0)</param>
	/// <param name="v2">2つ目の3次允E�Eクトル(t=1)</param>
	/// <param name="t">補間に使用する値(0~1)</param>
	/// <returns>線形補宁E/returns>
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
	/// ベジェ曲線を求めめE
	/// </summary>
	/// <param name="p0">始点</param>
	/// <param name="p1">制御点</param>
	/// <param name="p2">終点</param>
	/// <param name="t">媒介変数</param>
	/// <returns>ベジェ曲緁E/returns>
	[[nodiscard]] static Vector3 BezierCurve(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t);

	/// <summary>
	/// スプライン曲線を求めめE
	/// </summary>
	/// <param name="controlPoints"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	[[nodiscard]] static Vector3 CatmullRom(const std::vector<Vector3>& controlPoints, float t);

public:
	/// <summary>
	/// ワールド座標に変換する
	/// </summary>
	/// <param name="vector">変換允E��ーカル座樁E/param>
	/// <param name="matrix">変換に使用する行�E</param>
	/// <returns>ワールド座樁E/returns>
	[[nodiscard]] static Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

public:
	/// <summary>
	/// ある点からある点を見るrotationを返しまぁEラジアン)
	/// </summary>
	/// <param name="eyePosition">見る地点</param>
	/// <param name="targetPosition">見る対象</param>
	/// <returns>ラジアン</returns>
	[[nodiscard]] static Vector3 LookAt(const Vector3& eyePosition, const Vector3& targetPosition);

	/// <summary>
	/// 正封E��ベクトルを求めめE
	/// </summary>
	/// <param name="v1">1つ目の3次允E�Eクトル</param>
	/// <param name="v2">2つ目の3次允E�Eクトル</param>
	/// <returns>正封E��ベクトル</returns>
	[[nodiscard]] static Vector3 Project(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// あるベクトルの垂直なベクトルを求めめE
	/// </summary>
	/// <param name="vector">あるベクトル</param>
	/// <returns>あるベクトルの垂直なベクトル</returns>
	[[nodiscard]] static Vector3 Perpendicular(const Vector3& vector);

public:
	/// <summary>
	/// 琁E��座標系から直交座標系に変換する
	/// </summary>
	/// <param name="rtp">x=半征Ey=シータ,z=ファイ</param>
	/// <returns></returns>
	[[nodiscard]] static Vector3 SphericalToCartesian(const Vector3& rtp);
	/// <summary>
	/// チE��ルト座標系から琁E��座標系に変換する
	/// </summary>
	/// <param name="xyz"></param>
	/// <returns></returns>
	[[nodiscard]] static Vector3 CartesianToSpherical(const Vector3& xyz);
};

#endif // !VECTOR3_H
