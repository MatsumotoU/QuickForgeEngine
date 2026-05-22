#pragma once
#pragma once
#include <nlohmann/json.hpp>
#include "Transform.h"

#include "engine/include/core/Entity/Component/ComponentData.h"

namespace QFE {
	/**
	 * @class TransformComponent
	 * @brief 位置・回転・スケールのデータを保持し、行列変換などを行うコンポーネントクラス
	 */
	class TransformComponent final : public ComponentData {
	public:
		Transform transform;

		Matrix4x4 localMatrix; ///< ローカル変換行列

		/** @brief 行列から位置・回転・スケールを抽出 */
		void FromMatrix(const Matrix4x4& mat);
		/** @brief シリアライズ */
		nlohmann::json Serialize() const override;
		/** @brief デシリアライズ */
		void Deserialize(const nlohmann::json& json) override;
		/** @brief コンポーネントの型名を取得 */
		std::string GetTypeName() const override { return "Transform"; }
		/** @brief 前方に移動 */
		void AddForward(float distance);
		/** @brief 右方に移動 */
		void AddRight(float distance);

		bool operator==(const TransformComponent& other) const noexcept { return this == &other; }
		bool operator!=(const TransformComponent& other) const noexcept { return this != &other; }
		bool operator<(const TransformComponent& other) const noexcept { return this < &other; }
		bool operator<=(const TransformComponent& other) const noexcept { return this <= &other; }
		bool operator>(const TransformComponent& other) const noexcept { return this > &other; }
		bool operator>=(const TransformComponent& other) const noexcept { return this >= &other; }
	};
}
