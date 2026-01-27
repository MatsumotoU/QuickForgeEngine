/**
 * @file HierarchyView.h
 * @brief シーン内のエンティティ階層を表示・操作するパネルの実装
 */

#pragma once
#include "../IEditorUI.h"

#ifdef _DEBUG
#include "utility/DebugTool/ImGui/DropDownFileList.h"
#endif // _DEBUG
namespace QFE {
	/**
	 * @class HierarchyView
	 * @brief シーン上の全エンティティをリスト表示し、選択や親子付け、削除等を行うUI
	 */
	class HierarchyView final : public IEditorUI {
	public:
		HierarchyView();
		~HierarchyView() override = default;
		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;

		static uint32_t selectedEntityId_; ///< 現在選択されているエンティティID

	private:
		/** @brief コンテキストメニューの描画 */
		void DrawPopupContextWindow();
		/** @brief エンティティリストの描画 */
		void DrawEntityList();

#ifdef _DEBUG
		DropDownFileList modelDropDownFileList_; ///< モデル選択リスト
		DropDownFileList spriteDropDownFileList_; ///< スプライト選択リスト
		DropDownFileList entityDropDownFileList_; ///< エンティティテンプレート選択リスト
		int particleCount_ = 0; ///< パーティクル数管理用
#endif // _DEBUG
	};
}