/**
 * @file EngineProfileView.h
 * @brief エンジン全体のパフォーマンス統計（FPS等）を表示するパネル
 */

#pragma once
#include "../IEditorUI.h"

/**
 * @class EngineProfileView
 * @brief 基本的なFPSやメモリ使用状況などを簡易表示するUI
 */
class EngineProfileView : public IEditorUI {
public:
	EngineProfileView();
	~EngineProfileView() override = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;
};
