#pragma once
#include "IEditorCommand.h"
namespace QFE {
	/** @class LuaScriptInitializeCommand
	 * @brief Luaスクリプト環境の初期化を行うコマンド
	 */
	class LuaScriptInitializeCommand final : public IEditorCommand {
	public:
		// コンストラクタ
		LuaScriptInitializeCommand() = delete;
		explicit LuaScriptInitializeCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);
		~LuaScriptInitializeCommand() override = default;

		// Luaスクリプト環境の初期化を実行
		void Execute() override;
		// Undoは特に意味がないため空実装
		void Undo() override;
		// コマンド名を取得
		const std::vector<std::string> GetAliases() const override;

	private:
		const std::vector<std::string> aliases_;
	};
}