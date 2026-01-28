/**
 * @file PostprocessEdit.h
 * @brief ポストプロセス（ブルーム、色補正等）のパラメータを編集するパネル
 */

#pragma once
#include "../IEditorUI.h"
namespace QFE {
	/**
	 * @class PostprocessEdit
	 * @brief 各種レンダリングエフェクトのON/OFFや重み付けを調整するUI
	 */
	class PostprocessEdit final : public IEditorUI {
	public:
		PostprocessEdit() = default;
		~PostprocessEdit() = default;
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