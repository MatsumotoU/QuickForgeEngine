#include "EditorCommandList.h"
#include "IEditorCommand.h"

namespace QFE::EDITOR {
	void EditorCommandList::AddCommand(std::unique_ptr<IEditorCommand> command) {
		commandList_.emplace_back(std::move(command));
	}

	void EditorCommandList::ClearCommands() {
		commandList_.clear();
	}

	std::vector<std::unique_ptr<IEditorCommand>>& EditorCommandList::GetCommandList() {
		return commandList_;
	}
}