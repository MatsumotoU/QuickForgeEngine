/**
 * @file Transform.h
 * @brief エンティティの位置、回転、スケールを管理するコンポーネント
 */

#pragma once
#include <nlohmann/json.hpp>
#include "Vector/Vector3.h"
#include "Matrix/Matrix4x4.h"
#include "engine/include/core/Entity/Component/ComponentData.h"

/**
 * @class Transform
 * @brief 位置・回転・スケールのデータを保持し、行列変換などを行うコンポーネントクラス
 */
class Transform final : public ComponentData {
public:
    /// @brief スケール
    Vector3 scale{ 1.0f, 1.0f, 1.0f };
    /// @brief 回転（オイラー角）
    Vector3 rotate{ 0.0f, 0.0f, 0.0f };
    /// @brief 座標
    Vector3 translate{ 0.0f, 0.0f, 0.0f };

    /** @brief 行列から位置・回転・スケールを抽出 */
	void FromMatrix(const Matrix4x4& mat);
    /** @brief シリアライズ */
	nlohmann::json Serialize() const override;
    /** @brief デシリアライズ */
    void Deserialize(const nlohmann::json& json) override;
    /** @brief コンポーネントの型名を取得 */
    std::string GetTypeName() const override { return "Transform"; }

    /** @brief 前方に移動 */
    void AddForward(float distance) ;
    /** @brief 右方に移動 */
	void AddRight(float distance) ;

    bool operator==(const Transform& other) const noexcept { return this == &other; }
    bool operator!=(const Transform& other) const noexcept { return this != &other; }
    bool operator<(const Transform& other) const noexcept { return this < &other; }
    bool operator<=(const Transform& other) const noexcept { return this <= &other; }
    bool operator>(const Transform& other) const noexcept { return this > &other; }
    bool operator>=(const Transform& other) const noexcept { return this >= &other; }
};
