#pragma once
#include "../vector/Vector4.h"

namespace QFE::MATH {

	class Vector3;
	class Matrix4x4;

	class Quaternion final {
	public:
		Quaternion();

	public:
		Vector4 q;

	public:
		/// <summary>
		/// 虚数部を取得しまぁEx,y,z)
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] Vector3 ImaginaryPart() const;
		/// <summary>
		/// 実部を取得しまぁEw)
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] float RealPart() const;
		/// <summary>
		/// 共役を取得しまぁE
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] Quaternion Conjugation() const;
		/// <summary>
		/// ノルムを取得しまぁE
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] float Norm() const;
		/// <summary>
		/// 正規化クオータニオンを取得しまぁE
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] Quaternion Normalize() const;

	public:
		/// <summary>
		/// 積を求めまぁE
		/// </summary>
		/// <param name="lhs"></param>
		/// <param name="rhs"></param>
		/// <returns></returns>
		[[nodiscard]] static Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);
		/// <summary>
		/// 単位クオータニオンを返しまぁE
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] static Quaternion IndentityQuaternion();
		/// <summary>
		/// 共役クオータニオンを返しまぁE
		/// </summary>
		/// <param name="quaternion"></param>
		/// <returns></returns>
		[[nodiscard]] static Quaternion ConjugationQuaternion(const Quaternion& quaternion);
		/// <summary>
		/// クオータニオンのノルムを返しまぁE
		/// </summary>
		/// <param name="quaternion"></param>
		/// <returns></returns>
		[[nodiscard]] static float Norm(const Quaternion& quaternion);
		/// <summary>
		/// 正規化したクオータニオンを返しまぁE
		/// </summary>
		/// <param name="quaternion"></param>
		/// <returns></returns>
		[[nodiscard]] static Quaternion Normalize(const Quaternion& quaternion);
		/// <summary>
		/// 送Eオータニオンを返しまぁE
		/// </summary>
		/// <param name="quaternion"></param>
		/// <returns></returns>
		[[nodiscard]] static Quaternion Inverse(const Quaternion& quaternion);
		/// <summary>
		/// 任意E軸からクオータニオンを作EしまぁE
		/// </summary>
		/// <param name="axis"></param>
		/// <param name="angle"></param>
		/// <returns></returns>
		[[nodiscard]] static Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);
		/// <summary>
		/// クオータニオンでベクトルを回転させまぁE
		/// </summary>
		/// <param name="vector"></param>
		/// <param name="quaternion"></param>
		/// <returns></returns>
		[[nodiscard]] static Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion);
		/// <summary>
		/// クオータニオンから回転行Eを作EしまぁE
		/// </summary>
		/// <param name="quaternion"></param>
		/// <returns></returns>
		[[nodiscard]] static Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion);

		[[nodiscard]] static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);
	};

}
