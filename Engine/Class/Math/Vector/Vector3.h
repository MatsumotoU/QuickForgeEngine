#pragma once
#include <vector>

// 前方宣言
class Matrix4x4;

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
	/// ベジェ曲線を求める
	/// </summary>
	/// <param name="p0">始点</param>
	/// <param name="p1">制御点</param>
	/// <param name="p2">終点</param>
	/// <param name="t">媒介変数</param>
	/// <returns>ベジェ曲線</returns>
	[[nodiscard]] static Vector3 BezierCurve(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t);

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
};







