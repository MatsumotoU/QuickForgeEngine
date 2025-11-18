#pragma once
#include "../IEditorUI.h"
#include "AppUtility/DebugTool/ImGui/DropDownFileList.h"
#include <vector>
#include <string>

class InspectorView final : public IEditorUI {
public:
	InspectorView();
	~InspectorView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
private:
	uint32_t selectedEntityId_;
	DropDownFileList scriptList_;
	std::vector<std::string> csharpScriptClasses_;
	DropDownFileList modelList_;
	char scriptBuffer_[256];
	bool openScriptPopup_ = false;
};