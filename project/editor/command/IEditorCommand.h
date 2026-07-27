#pragma once
namespace QFE::EDITOR{
	/// @brief エディタコマンドのインターフェース
	class IEditorCommand {
		public:
			virtual ~IEditorCommand() = default;
			virtual void Execute() = 0;
			virtual void Undo() = 0;
	};
}
