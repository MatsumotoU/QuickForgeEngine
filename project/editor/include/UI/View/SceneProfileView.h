/**
 * @file SceneProfileView.h
 * @brief 現在のシーンに関する統計（エンティティ数、描画負荷等）を表示するパネル
 */

#pragma once
#include "../IEditorUI.h"
namespace QFE {
	/**
	 * @class SceneProfileView
	 * @brief シーン固有のオブジェクト数や処理時間などをモニタリングするUI
	 */
	class SceneProfileView : public IEditorUI {
	public:
		SceneProfileView();
		~SceneProfileView() override = default;
		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;
	};
}