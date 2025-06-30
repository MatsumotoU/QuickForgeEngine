#pragma once
#ifndef VECTOR4_H
#define VECTOR4_H

class Vector3;

class alignas(16) Vector4 final{
public:
	float x;
	float y;
	float z;
	float w;

public:
	Vector4 operator-()const { return { -x, -y, -z, -w }; };

	Vector4 operator+(const Vector4& other) const { return { x + other.x, y + other.y, z + other.z , w + other.w }; }
	Vector4 operator-(const Vector4& other) const { return { x - other.x, y - other.y, z - other.z , w - other.w }; }
	Vector4 operator*(const Vector4& other) const { return { x * other.x, y * other.y, z * other.z , w * other.w }; }
	Vector4 operator/(const Vector4& other) const { return { x / other.x, y / other.y, z / other.z , w / other.w }; }
	Vector4 operator=(const Vector4& other) { return { x = other.x, y = other.y, z = other.z , w = other.w }; }
	Vector4 operator+=(const Vector4& other) { return { x += other.x, y += other.y, z += other.z ,w += other.w }; }
	Vector4 operator-=(const Vector4& other) { return { x -= other.x, y -= other.y, z -= other.z ,w -= other.w }; }
	Vector4 operator*=(const Vector4& other) { return { x *= other.x, y *= other.y, z *= other.z ,w *= other.w }; }
	Vector4 operator/=(const Vector4& other) { return { x /= other.x, y /= other.y, z /= other.z ,w /= other.w }; }

	Vector4 operator+(const float& other) const { return { x + other,y + other,z + other , w + other}; };
	Vector4 operator-(const float& other) const { return { x - other,y - other,z - other , w - other }; };
	Vector4 operator/(const float& other) const { return { x / other,y / other,z / other , w / other }; };
	Vector4 operator*(const float& other) const { return { x * other,y * other,z * other , w * other }; };

public:
	/// <summary>
	/// 4次元ベクトルの長さを求める
	/// </summary>
	/// <returns>4次元ベクトルの長さ</returns>
	[[nodiscard]] float Length() const;

	/// <summary>
	/// 正規化された4次元ベクトルを求める
	/// </summary>
	/// <returns>正規化された4次元ベクトル</returns>
	[[nodiscard]] Vector4 Normalize() const;

	/// <summary>
	/// XYZの要素を抜き出します
	/// </summary>
	/// <returns></returns>
	[[nodiscard]] Vector3 xyz() const;

public:
	[[nodiscard]] static Vector4 Leap(const Vector4& v1, const Vector4& v2, float t);
};

#endif