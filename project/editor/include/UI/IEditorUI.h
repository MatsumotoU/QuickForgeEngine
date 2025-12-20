#pragma once
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>

class IEditorUI {
public:
	virtual ~IEditorUI() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Run() {};
	std::string GetName() const { return name_; }

	bool isActive_;
protected:
	std::string name_;
};
