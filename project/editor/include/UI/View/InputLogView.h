/**
 * @file InputLogView.h
 * @brief 入力イベントの履歴を記録し表示するパネル
 */

#pragma once
#include "../IEditorUI.h"
namespace QFE {
	/**
	 * @class InputLogView
	 * @brief 入力の発生順序やタイミングのデバッグに使用するログ表示UI
	 */
	class InputLogView final : public IEditorUI {
	public:
		InputLogView();
		~InputLogView() = default;

		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;
		/** @brief 実行時処理 */
		void Run() override;
	};
}