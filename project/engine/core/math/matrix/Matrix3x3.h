#pragma once
#ifndef Matrix3x3_H
#define Matrix3x3_H

namespace QFE::MATH {

	// 前方宣言
	class Vector2;

	class Matrix3x3 final {
	public:
		float m[3][3];

	public:
		float Get(int row, int col) const {
			return m[row][col];
		}
		void Set(int row, int col, float value) {
			m[row][col] = value;
		}

	public:
		Matrix3x3 operator+(const Matrix3x3& other);
		Matrix3x3 operator-(const Matrix3x3& other);
		Matrix3x3 operator*(const Matrix3x3& other);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 Inverse() const;

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 Transpose() const;

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix3x3 Inverse(const Matrix3x3& m);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix3x3 Transpose(const Matrix3x3& m);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix3x3 Add(const Matrix3x3& m1, const Matrix3x3& m2);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix3x3 Subtract(const Matrix3x3& m1, const Matrix3x3& m2);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix3x3 Multiply(const Matrix3x3& m1, const Matrix3x3& m2);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix3x3 MakeIdentity3x3();

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="scale">拡大縮小E惁EE</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 MakeScaleMatrix(const Vector2& scale) const;

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="theta">ラジアン</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 MakeRotateMatrix(const float& theta) const;

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="translate">移動量の惁EE</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 MakeTranslateMatrix(const Vector2& translate) const;

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="scale">拡大縮小E惁EE</param>
		/// <param name="rotate">ラジアン</param>
		/// <param name="translate">移動量の惁EE</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 MakeAffineMatrix(const Vector2& scale, const float& rotate, const Vector2& translate) const;

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="left">画面の左端座樁E/param>
		/// <param name="top">画面の上端座樁E/param>
		/// <param name="right">画面の右端座樁E/param>
		/// <param name="bottom">画面の下端座樁E/param>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom) const;

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="left">画面の左端座樁E/param>
		/// <param name="top">画面の上端座樁E/param>
		/// <param name="width">画面の右端座樁E/param>
		/// <param name="height">画面の下端座樁E/param>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height) const;

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="worldMatrix">入力値</param>
		/// <param name="viewMatrix">入力値</param>
		/// <param name="orthoMatrix">入力値</param>
		/// <param name="viewportMatrix">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix3x3 MakeWvpVpMatrix(const Matrix3x3& worldMatrix, const Matrix3x3& viewMatrix, const Matrix3x3& orthoMatrix, const Matrix3x3& viewportMatrix) const;
	};

}

#endif
