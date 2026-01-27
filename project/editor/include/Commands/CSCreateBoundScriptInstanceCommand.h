#pragma once
#include "IEditorCommand.h"
namespace QFE {
	class CSCreateBoundScriptInstanceCommand final : public IEditorCommand {
	public:
		explicit CSCreateBoundScriptInstanceCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

		void Execute() override;
		void Undo() override;
		const std::vector<std::string> GetAliases() const override;

	private:
		int entityId_;
		std::string className_;
		const std::vector<std::string> aliases_;
	};
}