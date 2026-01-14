/**
 * @file CreateNewScene.h
 * @brief 新しいシーンを作成するためのダイアログUI
 */

#pragma once
#include "../IEditorUI.h"

/**
 * @class CreateNewScene
 * @brief 空のシーンデータの作成、基本設定の初期化を行うUI
 */
class CreateNewScene final : public IEditorUI {
public:
	CreateNewScene() = default;
	~CreateNewScene() = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;
	/** @brief 実行時処理 */
	void Run() override;
private:
};
