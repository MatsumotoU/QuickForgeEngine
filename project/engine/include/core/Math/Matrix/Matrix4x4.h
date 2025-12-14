#pragma once
#ifndef Matrix4x4_H
#define Matrix4x4_H
#define _USE_MATH_DEFINES

#include <cmath>
#include <stdint.h>

// 前方宣言
class Vector3;
class Transform;

class Matrix4x4 final{
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
	/// こ�E行�Eの送E���Eを求めめE
	/// </summary>
	/// <returns>こ�E行�Eの送E���E</returns>
	[[nodiscard]] Matrix4x4 Inverse() const;

	/// <summary>
	/// こ�E行�Eの転地行�Eを求めめE
	/// </summary>
	/// <returns>こ�E行�Eの転地行�E</returns>
	[[nodiscard]] Matrix4x4 Transpose() const;

public:
	/// <summary>
	/// 4x4の送E���Eを求めめE
	/// </summary>
	/// <param name="m">4x4の行�E</param>
	/// <returns>4x4の送E���E</returns>
	[[nodiscard]] static Matrix4x4 Inverse(const Matrix4x4& m);

	/// <summary>
	/// 4x4の転置行�Eを求めめE
	/// </summary>
	/// <param name="m">4x4の行�E</param>
	/// <returns>4x4の転置行�E</returns>
	[[nodiscard]] static Matrix4x4 Transpose(const Matrix4x4& m);

public:
	/// <summary>
	/// 2つの4x4行�Eの加算結果を求めめE
	/// </summary>
	/// <param name="m1">1つ目の4x4行�E</param>
	/// <param name="m2">2つ目の4x4行�E</param>
	/// <returns>2つの4x4行�Eの加算結果</returns>
	[[nodiscard]] static Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

	/// <summary>
	/// 2つの4x4行�Eの減算結果を求めめE
	/// </summary>
	/// <param name="m1">1つ目の4x4行�E</param>
	/// <param name="m2">2つ目の4x4行�E</param>
	/// <returns>2つの4x4行�Eの減算結果</returns>
	[[nodiscard]] static Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

	/// <summary>
	/// 2つの4x4行�Eの乗算結果を求めめE
	/// </summary>
	/// <param name="m1">1つ目の4x4行�E</param>
	/// <param name="m2">2つ目の4x4行�E</param>
	/// <returns>2つの4x4行�Eの乗算結果</returns>
	[[nodiscard]] static Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

public:
	/// <summary>
	/// 4x4の単位行�Eを求めめE
	/// </summary>
	/// <returns>4x4の単位行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeIndentity4x4();

public:
	/// <summary>
	/// 拡大縮小行�Eを作�Eする
	/// </summary>
	/// <param name="scale">拡大縮小�E惁E��</param>
	/// <returns>拡大縮小行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeScaleMatrix(const Vector3& translate);

	/// <summary>
	/// 移動行�Eを作�Eする
	/// </summary>
	/// <param name="translate">移動量の惁E��</param>
	/// <returns>移動行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	/// <summary>
	/// X軸の回転行�Eを作�Eする
	/// </summary>
	/// <param name="radian">ラジアン</param>
	/// <returns>X軸の回転行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateXMatrix(const float& radian);

	/// <summary>
	/// Y軸の回転行�Eを作�Eする
	/// </summary>
	/// <param name="radian">ラジアン</param>
	/// <returns>X軸の回転行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateYMatrix(const float& radian);

	/// <summary>
	/// Z軸の回転行�Eを作�Eする
	/// </summary>
	/// <param name="radian">ラジアン</param>
	/// <returns>X軸の回転行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateZMatrix(const float& radian);

	/// <summary>
	/// 3次允E�E回転行�Eを作�Eする
	/// </summary>
	/// <param name="radian">ラジアンで構�Eされた回転惁E��</param>
	/// <returns>3次允E�E回転行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeRotateXYZMatrix(const Vector3& rotate);

	/// <summary>
	/// 3次允E�Eアフィン行�Eを作�Eする
	/// </summary>
	/// <param name="scale">拡縮の惁E��</param>
	/// <param name="rotate">ラジアン</param>
	/// <param name="translate">移動�E惁E��</param>
	/// <returns>3次允E�Eアフィン行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	[[nodiscard]] static Matrix4x4 MakeAffineMatrix(const Transform& transform);

	/// <summary>
	/// 任意軸回転行�Eを作�Eする
	/// </summary>
	/// <param name="axis">任意�E軸ベクトル</param>
	/// <param name="angle">ラジアン</param>
	/// <returns></returns>
	[[nodiscard]] static Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);

	/// <summary>
	/// ある点からある点を向く任意軸回転行�Eを作�Eする
	/// </summary>
	/// <param name="from">見る人</param>
	/// <param name="to">見られる人</param>
	/// <returns></returns>
	[[nodiscard]] static Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to);

public:
	/// <summary>
	/// 透視投影行�Eを作�Eする
	/// </summary>
	/// <param name="fovY">画见E/param>
	/// <param name="aspectRatio">アスペクト毁E/param>
	/// <param name="nearClip">近平面への距離</param>
	/// <param name="farClip">遠平面への距離</param>
	/// <returns>透視投影行�E</returns>
	[[nodiscard]] static Matrix4x4 MakePerspectiveFovMatrix(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip);

	/// <summary>
	/// 正規化チE��イス行�Eを作�Eする
	/// </summary>
	/// <param name="left">画面の左端</param>
	/// <param name="top">画面の上端</param>
	/// <param name="right">画面右端</param>
	/// <param name="bottom">画面の下端</param>
	/// <param name="nearClip">近平面への距離</param>
	/// <param name="farClip">遠平面への距離</param>
	/// <returns>正規化チE��イス行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip);
	
	/// <summary>
	/// ビューポ�Eト行�Eを作�Eする
	/// </summary>
	/// <param name="left">画面の左端</param>
	/// <param name="top">画面の上端</param>
	/// <param name="width">画面の横幁E/param>
	/// <param name="heigh">画面の縦幁E/param>
	/// <param name="minDepth">最小深度値</param>
	/// <param name="maxDepth">最大深度値</param>
	/// <returns>ビューポ�Eト行�E</returns>
	[[nodiscard]] static Matrix4x4 MakeViewportMatrix(const float& left, const float& top, const float& width, const float& heigh, const float& minDepth, const float& maxDepth);
};

#endif
