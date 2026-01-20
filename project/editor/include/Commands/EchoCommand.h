#pragma once
#include "IEditorCommand.h"

/** @class EchoCommand
 *  @brief コンソールにメッセージを表示するコマンド
 */
class EchoCommand final : public IEditorCommand {
public:
	// コマンドの別名一覧
	const static std::vector<std::string> aliases_;

	// コンストラクタ
	EchoCommand(std::vector<std::string>& consoleLog, const std::string& message)
		: IEditorCommand(consoleLog), message_(message) {
	}
	// コンソールにメッセージを表示
	void Execute() override;
	// Undoは特に意味がないため空実装
	void Undo() override;
	// コマンド名を取得
	const std::vector<std::string> GetAliases() const override;
private:
	std::string message_;
};