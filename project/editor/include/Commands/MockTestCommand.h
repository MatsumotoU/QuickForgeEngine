#pragma once
#include "IEditorCommand.h"
namespace QFE {
	/** @class ClearCommand
	 *  @brief コンソールのログをクリアするコマンド
	 */
	class MockTestCommand final : public IEditorCommand {
	public:
		// コンストラクタ
		explicit MockTestCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

		// テスト用のコマンド実行。実際には何も意味のない処理を行う
		void Execute() override;
		// Undoは特に意味がないため空実装
		void Undo() override;
		// コマンド名を取得
		const std::vector<std::string> GetAliases() const override;

	private:
		const std::vector<std::string> aliases_;
	};
}