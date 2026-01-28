/**
 * @file ConsoleView.h
 * @brief エンジンのログ出力を表示するコンソールパネル
 */

#pragma once
#include "../IEditorUI.h"
#ifdef QFE_OPTIMIZE_OFF
#include "utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
namespace QFE {
	/**
	 * @class ConsoleView
	 * @brief デバッグログ、警告、エラー等をリスト形式で表示するUI
	 */
	class ConsoleView : public IEditorUI {
	public:
		ConsoleView();
		~ConsoleView() override = default;
		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;
	private:
#ifdef QFE_OPTIMIZE_OFF
		LogLevel logLevel_; ///< 表示フィルタリング用のログレベル
#endif // _DEBUG
	};
}