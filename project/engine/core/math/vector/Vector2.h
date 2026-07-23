#pragma once

namespace QFE::MATH {

	class Vector2 final {
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
		Vector2& operator=(const Vector2& other) = default;
		Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
		Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
		Vector2& operator*=(const Vector2& other) { x *= other.x; y *= other.y; return *this; }
		Vector2& operator/=(const Vector2& other) { x /= other.x; y /= other.y; return *this; }

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
		/// 数学演算を行う。
		/// </summary>
		/// <param name="v1">1つ目のベクトル</param>
		/// <param name="v2">2つ目のベクトル</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static float Dot(const Vector2& v1, const Vector2& v2);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="v1">1つ目のベクトル</param>
		/// <param name="v2">2つ目のベクトル</param>
		/// <returns>2つのベクトルの外穁E/returns>
		[[nodiscard]] static float Cross(const Vector2& v1, const Vector2& v2);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="v1">1つ目のベクトル</param>
		/// <param name="v2">2つ目のベクトル</param>
		/// <returns>2つのベクトルの整数の距離</returns>
		[[nodiscard]] static float Distance(const Vector2& v1, const Vector2& v2);
	};

}
