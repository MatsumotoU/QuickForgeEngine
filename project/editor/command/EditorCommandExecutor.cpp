#include "EditorCommandExecutor.h"

#include "EditorCommandList.h"

QFE::EDITOR::EditorCommandExecutor::~EditorCommandExecutor() = default;

namespace {
	void ClearStack(std::stack<std::unique_ptr<QFE::EDITOR::IEditorCommand>>& stack) {
		while(!stack.empty()) {
			stack.pop();
		}
	}
}

void QFE::EDITOR::EditorCommandExecutor::Initialize() {
	ClearStack(undoStack_);
	ClearStack(redoStack_);
}

void QFE::EDITOR::EditorCommandExecutor::ExecuteCommand(EditorCommandList* command) {
	std::vector<std::unique_ptr<IEditorCommand>>& commandList = command->GetCommandList();
	// 何か新しくコマンドを実行する場合、redoStackをクリアする
	if(!commandList.empty()) {
		ClearStack(redoStack_);
	}

	// 入れられた順番でコマンドを実行する
	for (auto& cmd : command->GetCommandList()) {
		cmd->Execute();
		// 実行したコマンドをundoStackに追加する
		undoStack_.push(std::move(cmd));
	}
}

void QFE::EDITOR::EditorCommandExecutor::Undo() {
	if(undoStack_.empty()) {
		return;
	}
	// undoStackを実行して、redoStackに追加する
	undoStack_.top()->Undo();
	redoStack_.push(std::move(undoStack_.top()));
	undoStack_.pop();
}

void QFE::EDITOR::EditorCommandExecutor::Redo() {
	if(redoStack_.empty()) {
		return;
	}
	// redoStackを実行して、undoStackに追加する
	redoStack_.top()->Execute();
	undoStack_.push(std::move(redoStack_.top()));
	redoStack_.pop();
}
