#pragma once
#ifndef VECTOR4_H
#define VECTOR4_H

namespace QFE {

	class Vector3;
	class Matrix4x4;

	class  Vector4 final {
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

		Vector4 operator+(const float& other) const { return { x + other,y + other,z + other , w + other }; };
		Vector4 operator-(const float& other) const { return { x - other,y - other,z - other , w - other }; };
		Vector4 operator/(const float& other) const { return { x / other,y / other,z / other , w / other }; };
		Vector4 operator*(const float& other) const { return { x * other,y * other,z * other , w * other }; };

	public:
		/// <summary>
		/// 4次允Eクトル長さを求めめE
		/// </summary>
		/// <returns>4次允Eクトル長ぁE/returns>
		[[nodiscard]] float Length() const;

		/// <summary>
		/// 正規化されぁE次允Eクトルを求めめE
		/// </summary>
		/// <returns>正規化されぁE次允Eクトル</returns>
		[[nodiscard]] Vector4 Normalize() const;

		/// <summary>
		/// ワールド座標に変換する
		/// </summary>
		/// <param name="vector">変換允Eーカル座樁E/param>
		/// <param name="matrix">変換に使用する行E</param>
		/// <returns>ワールド座樁E/returns>
		[[nodiscard]] static Vector4 EulerTransform(const Vector4& vector, const Matrix4x4& matrix);

		/// <summary>
		/// XYZの要素を抜きEしまぁE
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] Vector3 xyz() const;

	public:
		[[nodiscard]] static Vector4 Leap(const Vector4& v1, const Vector4& v2, float t);
	};

}

#endif
