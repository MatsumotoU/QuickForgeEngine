/**
 * @file ConsoleView.h
 * @brief エンジンのログ出力を表示するコンソールパネル
 */

#pragma once
#include "../IEditorUI.h"
#ifdef QFE_OPTIMIZE_OFF
#include "utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
#include <unordered_map>
#include <functional>

namespace QFE {
	// ログの表示方法の種類を表す列挙型
	enum class LogKind {
		ByLevel,
		ByLocation
	};

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
		// ログレベル別の表示をする
		void DrawLogsByLevel();
		// クラス、関数ごとに分類されたログを表示する
		void DrawLogsByLocation();

		LogKind currentLogKind_; // 現在のログ表示方法
		std::unordered_map<LogKind, std::function<void()>> logDrawFunctions_;// ログ表示方法ごとの描画関数のマップ

#ifdef QFE_OPTIMIZE_OFF
		LogLevel logLevel_; ///< 表示フィルタリング用のログレベル
#endif // _DEBUG
	};
}