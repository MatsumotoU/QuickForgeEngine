#include "InspectorView.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"

InspectorView::InspectorView() {
	isActive_ = true;
	name_ = "Inspector View";
}

void InspectorView::Initialize() {
}

void InspectorView::Update() {
}

void InspectorView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(name_.c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	
	ImGui::End();
}
