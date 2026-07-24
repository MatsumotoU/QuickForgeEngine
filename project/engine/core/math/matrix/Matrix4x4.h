#pragma once
#ifndef Matrix4x4_H
#define Matrix4x4_H
#define _USE_MATH_DEFINES

#include <cmath>
#include <stdint.h>

namespace QFE::MATH {

	// 前方宣言
	class Vector3;
	class Vector4;
	class EulerTransform;

	class Matrix4x4 final {
	public:
		float m[4][4];

	public:
		float Get(int row, int col) const {
			return m[row][col];
		}
		void Set(int row, int col, float value) {
			m[row][col] = value;
		}

	public:
		Matrix4x4 operator+(const Matrix4x4& other) const;
		Matrix4x4 operator-(const Matrix4x4& other) const;
		Matrix4x4 operator*(const Matrix4x4& other) const;

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix4x4 Inverse() const;

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] Matrix4x4 Transpose() const;

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 Inverse(const Matrix4x4& m);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 Transpose(const Matrix4x4& m);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="m1">入力値</param>
		/// <param name="m2">入力値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeIdentity4x4();

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="scale">拡大縮小E惁EE</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeScaleMatrix(const Vector3& translate);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="translate">移動量の惁EE</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="radian">ラジアン</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeRotateXMatrix(const float& radian);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="radian">ラジアン</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeRotateYMatrix(const float& radian);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="radian">ラジアン</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeRotateZMatrix(const float& radian);

		/// <summary>
		/// 3次元の回転行列を作成する
		/// </summary>
		/// <param name="radian">ラジアンで構Eされた回転惁EE</param>
		/// <returns>3次元の回転行列</returns>
		[[nodiscard]] static Matrix4x4 MakeRotateXYZMatrix(const Vector3& rotate);

		/// <summary>
		/// 3次元のアフィン変換行列を作成する
		/// </summary>
		/// <param name="scale">拡縮の惁EE</param>
		/// <param name="rotate">ラジアン</param>
		/// <param name="translate">移動E惁EE</param>
		/// <returns>3次元のアフィン変換行列</returns>
		[[nodiscard]] static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
		[[nodiscard]] static Matrix4x4 MakeAffineMatrix(const Vector3& translate, const Vector4& rotate, const Vector3& scale);
		[[nodiscard]] static Matrix4x4 MakeAffineMatrix(const EulerTransform& transform);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="axis">任意E軸ベクトル</param>
		/// <param name="angle">ラジアン</param>
		/// <returns></returns>
		[[nodiscard]] static Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="from">見る人</param>
		/// <param name="to">見られる人</param>
		/// <returns></returns>
		[[nodiscard]] static Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to);

	public:
		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="fovY">画见E/param>
		/// <param name="aspectRatio">アスペクト毁E/param>
		/// <param name="nearClip">近平面への距離</param>
		/// <param name="farClip">遠平面への距離</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakePerspectiveFovMatrix(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="left">画面の左端</param>
		/// <param name="top">画面の上端</param>
		/// <param name="right">画面右端</param>
		/// <param name="bottom">画面の下端</param>
		/// <param name="nearClip">近平面への距離</param>
		/// <param name="farClip">遠平面への距離</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip);

		/// <summary>
		/// 数学演算を行う。
		/// </summary>
		/// <param name="left">画面の左端</param>
		/// <param name="top">画面の上端</param>
		/// <param name="width">画面の横幁E/param>
		/// <param name="heigh">画面の縦幁E/param>
		/// <param name="minDepth">最小深度値</param>
		/// <param name="maxDepth">最大深度値</param>
		/// <returns>計算結果</returns>
		[[nodiscard]] static Matrix4x4 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& heigh, const float& minDepth, const float& maxDepth);
	};

}

#endif
