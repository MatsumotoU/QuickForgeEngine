#pragma once

class Model;

/// @brief ステージセレクトシーンの天球
class StageSelectSkydome {
public:
	/// @brief コンストラクタ
	StageSelectSkydome() = default;

	/// @brief デストラクタ
	~StageSelectSkydome() = default;

	/// @brief 初期化
	/// @param model モデル
	void Initialize(Model *model);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

private:
	Model *model_ = nullptr;	// モデル
};