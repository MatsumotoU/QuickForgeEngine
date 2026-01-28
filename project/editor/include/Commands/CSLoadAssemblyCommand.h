#pragma once
#include "IEditorCommand.h"
namespace QFE {
	class CSLoadAssemblyCommand final : public IEditorCommand {
	public:
		explicit CSLoadAssemblyCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

		void Execute() override;
		void Undo() override;
		const std::vector<std::string> GetAliases() const override;

	private:
		const std::vector<std::string> aliases_;
	};
}