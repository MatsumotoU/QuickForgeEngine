#pragma once
#include <vector>
#include <string>
#include <memory>

namespace QFE::EDITOR {
	class IEditorCommand;

	/// @brief エディタコマンドのリストの所持を管理するクラス
	class EditorCommandList {
	public:
		/// @brief コマンドを追加する
		void AddCommand(std::unique_ptr<IEditorCommand> command);
		/// @brief コマンドを破棄する
		void ClearCommands();

		/// @brief コマンドリストを取得する
		std::vector<std::unique_ptr<IEditorCommand>>& GetCommandList();

	private:
		std::vector<std::unique_ptr<IEditorCommand>> commandList_;
	};
}
