#pragma once
#include "IEditorCommand.h"
namespace QFE {
	/** @class EchoCommand
	 *  @brief コンソールにメッセージを表示するコマンド
	 */
	class EchoCommand final : public IEditorCommand {
	public:
		// コンストラクタ
		EchoCommand() = delete;
		explicit EchoCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

		// コンソールにメッセージを表示
		void Execute() override;
		// Undoは特に意味がないため空実装
		void Undo() override;
		// コマンド名を取得
		const std::vector<std::string> GetAliases() const override;
	private:
		// コマンドの別名一覧
		const std::vector<std::string> aliases_;

		std::string message_;
	};
}