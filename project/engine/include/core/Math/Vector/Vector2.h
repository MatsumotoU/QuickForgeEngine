#pragma once

#ifndef VECTOR2_H
#define VECTOR2_H

class Vector2 final{
public:
	float x;
	float y;

	Vector2() : x(0.0f), y(0.0f) {}
	Vector2(float x_, float y_) : x(x_), y(y_) {}

public:
	Vector2 operator-()const { return { -x,-y }; };

	Vector2 operator+(const Vector2& other) const { return { x + other.x, y + other.y }; }
	Vector2 operator-(const Vector2& other) const { return { x - other.x, y - other.y }; }
	Vector2 operator*(const Vector2& other) const { return { x * other.x, y * other.y }; }
	Vector2 operator/(const Vector2& other) const { return { x / other.x, y / other.y }; }
	Vector2 operator=(const Vector2& other) { return { x = other.x, y = other.y }; }
	Vector2 operator+=(const Vector2& other) { return *this = *this + other; }
	Vector2 operator-=(const Vector2& other) { return *this = *this - other; }
	Vector2 operator*=(const Vector2& other) { return *this = *this * other; }
	Vector2 operator/=(const Vector2& other) { return *this = *this / other; }

	Vector2 operator+(const float& other) const { return { x + other,y + other }; };
	Vector2 operator-(const float& other) const { return { x - other,y - other }; };
	Vector2 operator/(const float& other) const { return { x / other,y / other }; };
	Vector2 operator*(const float& other) const { return { x * other,y * other }; };

public:
	/// <summary>
	/// 2次允E�Eクトルの長さを求めめE
	/// </summary>
	/// <returns>2次允E�Eクトルの長ぁE/returns>
	[[nodiscard]] float Length() const;

	/// <summary>
	/// 正規化されぁE次允E�Eクトルを求めめE
	/// </summary>
	/// <returns>正規化されぁE次允E�Eクトル</returns>
	[[nodiscard]] Vector2 Normalize() const;

public:
	/// <summary>
	/// 2つのベクトルの冁E��を求めめE
	/// </summary>
	/// <param name="v1">1つ目のベクトル</param>
	/// <param name="v2">2つ目のベクトル</param>
	/// <returns>2つのベクトルの冁E��E/returns>
	[[nodiscard]] static float Dot(const Vector2& v1, const Vector2& v2);

	/// <summary>
	/// 2つのベクトルの外積を求めめE
	/// </summary>
	/// <param name="v1">1つ目のベクトル</param>
	/// <param name="v2">2つ目のベクトル</param>
	/// <returns>2つのベクトルの外穁E/returns>
	[[nodiscard]] static float Cross(const Vector2& v1, const Vector2& v2);

	/// <summary>
	/// 2つのベクトルの距離を整数で求めめE
	/// </summary>
	/// <param name="v1">1つ目のベクトル</param>
	/// <param name="v2">2つ目のベクトル</param>
	/// <returns>2つのベクトルの整数の距離</returns>
	[[nodiscard]] static float Distance(const Vector2& v1, const Vector2& v2);
};

#endif // !VECTOR2.H
