#pragma once
#include "../vector/Vector3.h"
#include "../matrix/Matrix4x4.h"
#include "../quaternion/Quaternion.h"

namespace QFE::MATH {

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

		bool operator==(const EulerTransform& other) const noexcept = default;
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
