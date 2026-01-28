#pragma once
#include "IEditorCommand.h"
namespace QFE {
	class CSCreateProjectCommand final : public IEditorCommand {
	public:
		explicit CSCreateProjectCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

		void Execute() override;
		void Undo() override;
		const std::vector<std::string> GetAliases() const override;

	private:
		std::string projectName_;
		const std::vector<std::string> aliases_;
	};
}