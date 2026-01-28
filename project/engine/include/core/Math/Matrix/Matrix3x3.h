#pragma once
#ifndef Matrix3x3_H
#define Matrix3x3_H

namespace QFE {

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
		/// こE行Eの送EEを求めめE
		/// </summary>
		/// <returns>こE行Eの送EE</returns>
		[[nodiscard]] Matrix3x3 Inverse() const;

		/// <summary>
		/// こE行Eの転地行Eを求めめE
		/// </summary>
		/// <returns>こE行Eの転地行E</returns>
		[[nodiscard]] Matrix3x3 Transpose() const;

	public:
		/// <summary>
		/// 3x3の送EEを求めめE
		/// </summary>
		/// <param name="m">3x3の行E</param>
		/// <returns>3x3の送EE</returns>
		[[nodiscard]] static Matrix3x3 Inverse(const Matrix3x3& m);

		/// <summary>
		/// 3x3の転置行Eを求めめE
		/// </summary>
		/// <param name="m">3x3の行E</param>
		/// <returns>3x3の転置行E</returns>
		[[nodiscard]] static Matrix3x3 Transpose(const Matrix3x3& m);

	public:
		/// <summary>
		/// 2つの3x3行Eの加算結果を求めめE
		/// </summary>
		/// <param name="m1">1つ目の3x3行E</param>
		/// <param name="m2">2つ目の3x3行E</param>
		/// <returns>2つの3x3行Eの加算結果</returns>
		[[nodiscard]] static Matrix3x3 Add(const Matrix3x3& m1, const Matrix3x3& m2);

		/// <summary>
		/// 2つの3x3行Eの減算結果を求めめE
		/// </summary>
		/// <param name="m1">1つ目の3x3行E</param>
		/// <param name="m2">2つ目の3x3行E</param>
		/// <returns>2つの3x3行Eの減算結果</returns>
		[[nodiscard]] static Matrix3x3 Subtract(const Matrix3x3& m1, const Matrix3x3& m2);

		/// <summary>
		/// 2つの3x3行Eの乗算結果を求めめE
		/// </summary>
		/// <param name="m1">1つ目の3x3行E</param>
		/// <param name="m2">2つ目の3x3行E</param>
		/// <returns>2つの3x3行Eの乗算結果</returns>
		[[nodiscard]] static Matrix3x3 Multiply(const Matrix3x3& m1, const Matrix3x3& m2);

	public:
		/// <summary>
		/// 3x3の単位行Eを求めめE
		/// </summary>
		/// <returns>3x3の単位行E</returns>
		[[nodiscard]] static Matrix3x3 MakeIdentity3x3();

	public:
		/// <summary>
		/// 拡大縮小行Eを作Eする
		/// </summary>
		/// <param name="scale">拡大縮小E惁EE</param>
		/// <returns>拡大縮小行E</returns>
		[[nodiscard]] Matrix3x3 MakeScaleMatrix(const Vector2& scale) const;

		/// <summary>
		/// 回転行Eを作Eする
		/// </summary>
		/// <param name="theta">ラジアン</param>
		/// <returns>回転行E</returns>
		[[nodiscard]] Matrix3x3 MakeRotateMatrix(const float& theta) const;

		/// <summary>
		/// 移動行Eを作Eする
		/// </summary>
		/// <param name="translate">移動量の惁EE</param>
		/// <returns>移動行E</returns>
		[[nodiscard]] Matrix3x3 MakeTranslateMatrix(const Vector2& translate) const;

		/// <summary>
		/// アフィン行Eを作Eする
		/// </summary>
		/// <param name="scale">拡大縮小E惁EE</param>
		/// <param name="rotate">ラジアン</param>
		/// <param name="translate">移動量の惁EE</param>
		/// <returns>アフィン行E</returns>
		[[nodiscard]] Matrix3x3 MakeAffineMatrix(const Vector2& scale, const float& rotate, const Vector2& translate) const;

	public:
		/// <summary>
		/// 正封EE行Eを作Eする
		/// </summary>
		/// <param name="left">画面の左端座樁E/param>
		/// <param name="top">画面の上端座樁E/param>
		/// <param name="right">画面の右端座樁E/param>
		/// <param name="bottom">画面の下端座樁E/param>
		/// <returns>正封EE行E</returns>
		[[nodiscard]] Matrix3x3 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom) const;

		/// <summary>
		/// ビューポEト行Eを作Eする
		/// </summary>
		/// <param name="left">画面の左端座樁E/param>
		/// <param name="top">画面の上端座樁E/param>
		/// <param name="width">画面の右端座樁E/param>
		/// <param name="height">画面の下端座樁E/param>
		/// <returns>ビューポEト行E</returns>
		[[nodiscard]] Matrix3x3 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height) const;

		/// <summary>
		/// ワールドビュープロジェクション行Eを作Eする
		/// </summary>
		/// <param name="worldMatrix">ワールド行E</param>
		/// <param name="viewMatrix">ビュー行E</param>
		/// <param name="orthoMatrix">正封EE行E</param>
		/// <param name="viewportMatrix">ビューポEト行E</param>
		/// <returns>ワールドビュープロジェクション行E</returns>
		[[nodiscard]] Matrix3x3 MakeWvpVpMatrix(const Matrix3x3& worldMatrix, const Matrix3x3& viewMatrix, const Matrix3x3& orthoMatrix, const Matrix3x3& viewportMatrix) const;
	};

}

#endif
