#pragma once
#include "IEditorUI.h"

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

private:
	void DrawPopupContextWindow();
	void DrawEntityList();
#ifdef _DEBUG
	DropDownFileList modelDropDownFileList_;
#endif // _DEBUG
};