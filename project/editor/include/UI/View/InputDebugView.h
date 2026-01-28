/**
 * @file InputDebugView.h
 * @brief キーボード、マウス、ゲームパッドの入力を可視化するデバッグパネル
 */

#pragma once
#include "../IEditorUI.h"
namespace QFE {
	/**
	 * @class InputDebugView
	 * @brief 入力デバイスの各ボタン、軸の状態をリアルタイムに表示するUI
	 */
	class InputDebugView final : public IEditorUI {
	public:
		InputDebugView();
		~InputDebugView() = default;

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
}