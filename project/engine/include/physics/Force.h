/**
 * @file Force.h
 * @brief 物理挙動を制御するためのコンポーネント
 */

#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"
#include "engine/include/core/Math/Vector/Vector3.h"

/**
 * @class Force
 * @brief 速度、加速度、質量、摩擦、重力などの物理パラメータを保持し計算するコンポーネントクラス
 */
class Force final : public ComponentData {
public:
    /// @brief 現在の速度
	Vector3 velocity;
    /// @brief 現在の加速度
	Vector3 acceleration;
    /// @brief 質量
	float mass;
    /// @brief 摩擦係数
	float friction;
    /// @brief 重力の強さ
	float gravityStrength;
    /// @brief 重力を適用するかどうか
	bool isGravity;

	Force();
	~Force() override = default;

    /**
     * @brief 力を加える
     * @param force 加える力のベクトル
     */
	void AddForce(const Vector3& force);

    /** @brief シリアライズ */
	nlohmann::json Serialize() const override;
    /** @brief デシリアライズ */
	void Deserialize(const nlohmann::json& json) override;
    /** @brief 型名を取得 */
	std::string GetTypeName() const override;
};
