#include "HierarchyView.h"

HierarchyView::HierarchyView() {
	isActive_ = true;
	name_ = "Hierarchy View";
}

void HierarchyView::Initialize() {
}

void HierarchyView::Update() {
}

void HierarchyView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(name_.c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

	ImGui::End();
}
