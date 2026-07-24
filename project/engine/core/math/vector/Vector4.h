#pragma once
namespace QFE::MATH {

	class Vector3;
	class Matrix4x4;

	class Vector4 final {
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
		Vector4& operator=(const Vector4& other) = default;
		Vector4& operator+=(const Vector4& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
		Vector4& operator-=(const Vector4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
		Vector4& operator*=(const Vector4& other) { x *= other.x; y *= other.y; z *= other.z; w *= other.w; return *this; }
		Vector4& operator/=(const Vector4& other) { x /= other.x; y /= other.y; z /= other.z; w /= other.w; return *this; }

		Vector4 operator+(const float& other) const { return { x + other,y + other,z + other , w + other }; };
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
		/// ワールド座標に変換する
		/// </summary>
		/// <param name="vector">変換するローカル座標</param>
		/// <param name="matrix">入力値</param>
		/// <returns>ワールド座樁E/returns>
		[[nodiscard]] static Vector4 EulerTransform(const Vector4& vector, const Matrix4x4& matrix);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] Vector3 xyz() const;

	public:
		[[nodiscard]] static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t);
	};

}
