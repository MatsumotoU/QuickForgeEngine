/**
 * @file ColliderMaskEdit.h
 * @brief コリジョンレイヤーやマスクの設定を編集するパネル
 */

#pragma once
#include "../IEditorUI.h"
namespace QFE {
	/**
	 * @class ColliderMaskEdit
	 * @brief 特定のタグ同士の衝突を許可・禁止するなどの判定設定を行うUI
	 */
	class ColliderMaskEdit final : public IEditorUI {
	public:
		ColliderMaskEdit();
		~ColliderMaskEdit() = default;
		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;
		/** @brief 実行時処理 */
		void Run() override;
	private:
		char tag1Buf_[64] = {}; ///< タグ1入力バッファ
		char tag2Buf_[64] = {}; ///< タグ2入力バッファ
	};
}