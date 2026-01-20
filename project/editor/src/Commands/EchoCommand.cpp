#include "editor/include/Commands/EchoCommand.h"

// コマンドの別名一覧の定義
EchoCommand::EchoCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "echo", "print", "msg" }) { }

void EchoCommand::Execute() {
	if (!command_) return;
	for (auto& alias : aliases_) {
		if (strncmp(command_, alias.c_str(), alias.size()) == 0) {
			message_ = command_ + alias.length();
			if (message_.length() > 0 && message_[0] == ' ') {
				message_ = message_.substr(1);
			}
			cons_.emplace_back(message_);
			return;
		}
	}
}

void EchoCommand::Undo() {
	// EchoコマンドのUndoは特に意味がないため空実装としています
}

const std::vector<std::string> EchoCommand::GetAliases() const {
	return aliases_;
}
