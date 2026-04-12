#pragma once
#include "IEditorCommand.h"
namespace QFE {
	/** @class ClearCommand
	 *  @brief コンソールのログをクリアするコマンド
	 */
	class BinarizeSceneCommand final : public IEditorCommand {
	public:
		// コンストラクタ
		explicit BinarizeSceneCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

		// コンソールのログをクリア
		void Execute() override;
		// Undoは特に意味がないため空実装
		void Undo() override;
		// コマンド名を取得
		const std::vector<std::string> GetAliases() const override;

	private:
		const std::vector<std::string> aliases_;
	};
}