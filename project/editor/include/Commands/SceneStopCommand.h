#pragma once
#include "IEditorCommand.h"

class SceneStopCommand final : public IEditorCommand {
public:
	explicit SceneStopCommand(std::vector<std::string>& consoleLog, const char* command = nullptr);

	void Execute() override;
	void Undo() override;
	const std::vector<std::string> GetAliases() const override;

private:
	const std::vector<std::string> aliases_;
};
