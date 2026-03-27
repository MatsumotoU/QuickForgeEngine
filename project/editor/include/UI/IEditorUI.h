/**
 * @file IEditorUI.h
 * @brief エディタ固有のUIパネル・ウィンドウの基本インターフェース
 */

#pragma once
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>
namespace QFE {
	/**
	 * @class IEditorUI
	 * @brief 各種エディタUI（インスペクタ、ヒエラルキー等）の基底クラス
	 */
	class IEditorUI {
	public:
		virtual ~IEditorUI() = default;
		/** @brief 初期化 */
		virtual void Initialize() = 0;
		/** @brief 更新 */
		virtual void Update() = 0;
		/** @brief 描画(ImGui) */
		virtual void Draw() = 0;
		/** @brief 実行時処理 */
		virtual void Run() {};
		/** @brief UI名の取得 */
		std::string GetName() const { return name_; }
		/** @brief UI名の設定 */
		void SetName(const std::string& name) { name_ = name; }

		bool isActive_; ///< アクティブ状態
	private:
		std::string name_; ///< UI名
	};
}