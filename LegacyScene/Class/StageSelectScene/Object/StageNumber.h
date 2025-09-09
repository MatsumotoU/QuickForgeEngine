#pragma once
#include "../Engine/Math/Matrix/Matrix4x4.h"

class Model;

/// @brief ステージ番号
class StageNumber {
public:
	static inline constexpr float kRotateTime = 4.0f;	// 回転にかける時間(秒)
	static inline constexpr float kHeightOffset = 1.2f;	// 高さオフセット

	/// @brief コンストラクタ
	StageNumber() = default;

	/// @brief デストラクタ
	~StageNumber() = default;

	/// @brief 初期化
	/// @param model モデル
	/// @param parentWorldMatrix 親のワールド行列
	void Initialize(Model *model, const Matrix4x4 &parentWorldMatrix);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief リセット
	void Reset();

	/// @brief 親のワールド行列を設定
	/// @param parentWorldMatrix 親のワールド行列
	void SetParentWorldMatrix(const Matrix4x4 &parentWorldMatrix) { parentWorldMatrix_ = parentWorldMatrix; }

private:
	Model *model_ = nullptr;		// モデル
	Matrix4x4 parentWorldMatrix_{};	// 親のワールド行列
	float theta_ = 0.0f;			// アニメーション用角度
};