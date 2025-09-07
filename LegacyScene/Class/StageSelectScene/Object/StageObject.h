#pragma once
#include <cstdint>

class Model;

/// @brief ステージオブジェクト
class StageObject {
public:
	/// @brief コンストラクタ
	StageObject() = default;

	/// @brief デストラクタ
	~StageObject() = default;

	/// @brief 初期化
	/// @param model モデル
	/// @param stageNumber ステージ番号
	void Initialize(Model *model, uint32_t stageNumber);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief リセット
	void Reset();

private:
	Model* model_ = nullptr;	// モデル
	float theta_ = 0.0f;		// アニメーション用角度
};