/**
 * @file GameView.h
 * @brief ゲームの最終的なレンダリング結果のみを表示する純粋な描画パネル
 */

#pragma once
#include "../IEditorUI.h"
namespace QFE {
	/**
	 * @class GameView
	 * @brief エディタのUIを除いた、ゲーム単体での見え方を確認するためのUI
	 */
	class GameView final : public IEditorUI {
	public:
		GameView();
		~GameView() override = default;
		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;
	private:
		bool isSceneViewFocused_; ///< シーンビューがフォーカスされているか（入力制御用）
	};
}