#pragma once
#include "IEditorCommand.h"

class SceneRunCommand final : public IEditorCommand {
public:
	explicit SceneRunCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

	void Execute() override;
	void Undo() override;
	const std::vector<std::string> GetAliases() const override;

private:
	const std::vector<std::string> aliases_;
};
