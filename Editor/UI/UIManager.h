#pragma once
#include "IEditorUI.h"
#include <memory>
#include <vector>

class UIManager final {
public:
	UIManager() = default;
	~UIManager() = default;
	void Initialize();
	void Update();
	void Draw();
private:
	bool isActiveUI_;
	std::vector<std::unique_ptr<IEditorUI>> editorUIs_;
};
