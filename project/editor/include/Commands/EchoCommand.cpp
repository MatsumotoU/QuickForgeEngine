#include "editor/include/Commands/EchoCommand.h"

// コマンドの別名一覧の定義
const std::vector<std::string> EchoCommand::aliases_ = { "echo", "print", "msg" };

void EchoCommand::Execute() {
	cons_.emplace_back(message_);
}

void EchoCommand::Undo() {
	// EchoコマンドのUndoは特に意味がないため空実装としています
}

const std::vector<std::string> EchoCommand::GetAliases() const {
	return aliases_;
}
