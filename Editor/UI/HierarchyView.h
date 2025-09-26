#pragma once
#include "IEditorUI.h"
class HierarchyView final : public IEditorUI {
public:
	HierarchyView();
	~HierarchyView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
private:
};