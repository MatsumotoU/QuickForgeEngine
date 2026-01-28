/**
 * @file ScriptLoggerView.h
 * @brief スクリプト（Lua/C#）からの出力を表示するパネル
 */

#pragma once
#include "../IEditorUI.h"
#include "core/Math/Vector/Vector3.h"
namespace QFE {
	/**
	 * @class ScriptLoggerView
	 * @brief スクリプト実行中のprint出力やエラーをキャッチして表示するUI
	 */
	class ScriptLoggerView : public IEditorUI {
	public:
		ScriptLoggerView();
		~ScriptLoggerView() override = default;
		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;

	private:
		uint32_t selectedEntityId_; ///< 選択されているエンティティID
	};
}