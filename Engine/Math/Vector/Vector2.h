#pragma once
class Vector2 final{
public:
	float x;
	float y;

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
	/// 2次元ベクトルの長さを求める
	/// </summary>
	/// <returns>2次元ベクトルの長さ</returns>
	[[nodiscard]] float Length() const;

	/// <summary>
	/// 正規化された2次元ベクトルを求める
	/// </summary>
	/// <returns>正規化された2次元ベクトル</returns>
	[[nodiscard]] Vector2 Normalize() const;

public:
	/// <summary>
	/// 2つのベクトルの内積を求める
	/// </summary>
	/// <param name="v1">1つ目のベクトル</param>
	/// <param name="v2">2つ目のベクトル</param>
	/// <returns>2つのベクトルの内積</returns>
	[[nodiscard]] static float Dot(const Vector2& v1, const Vector2& v2);

	/// <summary>
	/// 2つのベクトルの外積を求める
	/// </summary>
	/// <param name="v1">1つ目のベクトル</param>
	/// <param name="v2">2つ目のベクトル</param>
	/// <returns>2つのベクトルの外積</returns>
	[[nodiscard]] static float Cross(const Vector2& v1, const Vector2& v2);

	/// <summary>
	/// 2つのベクトルの距離を整数で求める
	/// </summary>
	/// <param name="v1">1つ目のベクトル</param>
	/// <param name="v2">2つ目のベクトル</param>
	/// <returns>2つのベクトルの整数の距離</returns>
	[[nodiscard]] static float Distance(const Vector2& v1, const Vector2& v2);
};