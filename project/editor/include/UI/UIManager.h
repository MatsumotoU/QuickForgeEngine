/**
 * @file UIManager.h
 * @brief エディタ内の全てのUIコンポーネントを管理するクラス
 */

#pragma once
#include "IEditorUI.h"
#include <memory>
#include <vector>
namespace QFE {
	/**
	 * @class UIManager
	 * @brief 各種エディタUIのリストを保持し、一括で更新・描画を行う。
	 */
	class UIManager final {
	public:
		UIManager() = default;
		~UIManager() = default;
		/** @brief 初期化(各UIの生成) */
		void Initialize();
		/** @brief 更新 */
		void Update();
		/** @brief 描画 */
		void Draw();
	private:
		bool isActiveUI_; ///< UI全体がアクティブかどうか
		std::vector<std::unique_ptr<IEditorUI>> fileUIs_; ///< メニューバーなどのファイル操作系UI
		std::vector<std::unique_ptr<IEditorUI>> viewUIs_; ///< シーンビュー、ヒエラルキーなどの表示系UI
		std::vector<std::unique_ptr<IEditorUI>> editUIs_; ///< インスペクタなどの編集系UI
	};
}