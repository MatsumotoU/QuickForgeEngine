#pragma once
#include "../IEditorUI.h"
#include <vector>

class DebugConsole final : public IEditorUI {
public:
	DebugConsole() = default;
	~DebugConsole() = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
private:
	char inputBuf_[256] = {};
	std::vector<std::string> items_;

	void ExecCommand(const char* command);
};
