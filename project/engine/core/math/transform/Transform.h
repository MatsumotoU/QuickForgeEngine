#pragma once
#include "../vector/Vector3.h"
#include "../matrix/Matrix4x4.h"

namespace QFE::MATH {
	/// @brief 位置、回転、スケールを表すクラス
	class Transform final {
	public:
		/// @brief スケール
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		/// @brief 回転（オイラー角）
		Vector3 rotate{ 0.0f, 0.0f, 0.0f };
		/// @brief 座標
		Vector3 translate{ 0.0f, 0.0f, 0.0f };

		/** @brief 行列から位置・回転・スケールを抽出 */
		void FromMatrix(const Matrix4x4& mat);

		bool operator==(const Transform& other) const noexcept { return this == &other; }
		bool operator!=(const Transform& other) const noexcept { return this != &other; }
		bool operator<(const Transform& other) const noexcept { return this < &other; }
		bool operator<=(const Transform& other) const noexcept { return this <= &other; }
		bool operator>(const Transform& other) const noexcept { return this > &other; }
		bool operator>=(const Transform& other) const noexcept { return this >= &other; }
	};

}
