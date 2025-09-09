#pragma once
#include "../Engine/Math/Transform.h"
#include "../Engine/Math/Matrix/Matrix4x4.h"

/// @brief ステージ選択の基準点
class StageSelectAnchor {
public:
	/// @brief コンストラクタ
	StageSelectAnchor() = default;

	/// @brief デストラクタ
	~StageSelectAnchor() = default;

	/// @brief 初期化
	/// @param stageNumber ステージ番号
	void Initialize(uint32_t stageNumber);

	/// @brief ワールド行列を取得する
	/// @return ワールド行列
	Matrix4x4 GetWorldMatrix() const { return worldMatrix_; }

	/// @brief ワールド座標を取得する
	/// @return ワールド座標
	Vector3 GetWorldPosition() const { return transform_.translate; }

private:
	Transform transform_;	// ワールド変換データ
	Matrix4x4 worldMatrix_;	// ワールド行列
};