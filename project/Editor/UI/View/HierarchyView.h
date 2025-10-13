#pragma once
#include "../IEditorUI.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/ImGui/DropDownFileList.h"
#endif // _DEBUG

class HierarchyView final : public IEditorUI {
public:
	HierarchyView();
	~HierarchyView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;

	static uint32_t selectedEntityId_;

private:
	void DrawPopupContextWindow();
	void DrawEntityList();

#ifdef _DEBUG
	DropDownFileList modelDropDownFileList_;
	DropDownFileList spriteDropDownFileList_;
	DropDownFileList entityDropDownFileList_;
#endif // _DEBUG
};