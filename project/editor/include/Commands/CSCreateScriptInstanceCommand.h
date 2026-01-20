#pragma once
#include "IEditorCommand.h"

class CSCreateScriptInstanceCommand final : public IEditorCommand {
public:
	explicit CSCreateScriptInstanceCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

	void Execute() override;
	void Undo() override;
	const std::vector<std::string> GetAliases() const override;

private:
	std::string className_;
	const std::vector<std::string> aliases_;
};
