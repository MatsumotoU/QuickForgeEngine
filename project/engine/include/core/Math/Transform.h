/**
 * @file EulerTransform.h
 * @brief エンティティの位置、回転、スケールを管理するコンポーネント
 */

#pragma once
#include "Vector/Vector3.h"
#include "Matrix/Matrix4x4.h"
#include "Quaternion/Quaternion.h"

namespace QFE {

	/**
	 * @class EulerTransform
	 * @brief 位置・回転・スケールのデータを保持し、行列変換などを行うコンポーネントクラス
	 */
	class EulerTransform final {
	public:
		/// @brief スケール
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		/// @brief 回転（オイラー角）
		Vector3 rotate{ 0.0f, 0.0f, 0.0f };
		/// @brief 座標
		Vector3 translate{ 0.0f, 0.0f, 0.0f };

		/** @brief 行列から位置・回転・スケールを抽出 */
		void FromMatrix(const Matrix4x4& mat);

		bool operator==(const EulerTransform& other) const noexcept { return this == &other; }
		bool operator!=(const EulerTransform& other) const noexcept { return this != &other; }
		bool operator<(const EulerTransform& other) const noexcept { return this < &other; }
		bool operator<=(const EulerTransform& other) const noexcept { return this <= &other; }
		bool operator>(const EulerTransform& other) const noexcept { return this > &other; }
		bool operator>=(const EulerTransform& other) const noexcept { return this >= &other; }
	};

	class QuaternionTransform final {
	public:
		/// @brief スケール
		Vector3 scale;
		/// @brief 回転
		Quaternion rotate;
		/// @brief 座標
		Vector3 translate;
	};

}
