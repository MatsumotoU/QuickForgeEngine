#include "EditorCommandExecutor.h"

#include "EditorCommandList.h"

QFE::EDITOR::EditorCommandExecutor::~EditorCommandExecutor() = default;

void QFE::EDITOR::EditorCommandExecutor::Initialize() {
	while(!undoQueue_.empty()) {
		undoQueue_.pop();
	}
	while(!redoQueue_.empty()) {
		redoQueue_.pop();
	}
}

void QFE::EDITOR::EditorCommandExecutor::ExecuteCommand(EditorCommandList* command) {
	std::vector<std::unique_ptr<IEditorCommand>>& commandList = command->GetCommandList();
	// 何か新しくコマンドを実行する場合、redoQueueをクリアする
	if(!commandList.empty()) {
		while(!redoQueue_.empty()) {
			redoQueue_.pop();
		}
	}

	// 入れられた順番でコマンドを実行する
	for (auto& cmd : command->GetCommandList()) {
		cmd->Execute();
		// 実行したコマンドをundoQueueに追加する
		undoQueue_.push(std::move(cmd));
	}
}

void QFE::EDITOR::EditorCommandExecutor::Undo() {
	if(undoQueue_.empty()) {
		return;
	}
	// undoQueueを実行して、redoQueueに追加する
	undoQueue_.front()->Undo();
	redoQueue_.push(std::move(undoQueue_.front()));
	undoQueue_.pop();
}

void QFE::EDITOR::EditorCommandExecutor::Redo() {
	if(redoQueue_.empty()) {
		return;
	}
	// redoQueueを実行して、undoQueueに追加する
	redoQueue_.front()->Execute();
	undoQueue_.push(std::move(redoQueue_.front()));
	redoQueue_.pop();
}
