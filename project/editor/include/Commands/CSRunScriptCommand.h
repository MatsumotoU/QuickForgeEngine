#pragma once
#include "IEditorCommand.h"
namespace QFE {
	class CSRunScriptCommand final : public IEditorCommand {
	public:
		explicit CSRunScriptCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

		void Execute() override;
		void Undo() override;
		const std::vector<std::string> GetAliases() const override;

	private:
		int index_;
		std::string functionName_;
		const std::vector<std::string> aliases_;
	};
}