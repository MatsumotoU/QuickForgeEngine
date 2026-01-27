#include "editor/include/Commands/ClearCommand.h"
using namespace QFE;
ClearCommand::ClearCommand(std::vector<std::string>& consoleLog, const char* command) 
	: IEditorCommand(consoleLog, command),
	aliases_({"clear","cls"}) { }

void ClearCommand::Execute() {
	// コマンドが別名のいずれかと一致する場合、コンソールをクリア
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.clear();
			return;
		}
	}
}

void ClearCommand::Undo() {
	// ClearコマンドのUndoは実装が難しいため（履歴をすべて保存する必要がある）、空実装とします
}

const std::vector<std::string> ClearCommand::GetAliases() const {
	return aliases_;
}
