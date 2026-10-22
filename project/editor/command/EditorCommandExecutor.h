#pragma once
#include <stack>
#include <memory>
#include "IEditorCommand.h"

namespace QFE::EDITOR {
	class EditorCommandList;

	/// @brief エディタコマンドの実行を管理するクラス
	class EditorCommandExecutor {
	public:
		~EditorCommandExecutor();

		/// @brief 初期化処理
		void Initialize();

		/// @brief コマンドを実行する
		void ExecuteCommand(EditorCommandList* command);
		/// @brief コマンドを元に戻す
		void Undo();
		/// @brief コマンドをやり直す
		void Redo();

	private:
		std::stack<std::unique_ptr<IEditorCommand>> undoStack_;
		std::stack<std::unique_ptr<IEditorCommand>> redoStack_;
	};
}
