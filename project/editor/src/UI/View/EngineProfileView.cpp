#include "EngineProfileView.h"
#include "core/EngineGlobalValue.h"

EngineProfileView::EngineProfileView() {
	isActive_ = false;
	name_ = "Engine Profile View";
}

void EngineProfileView::Initialize() {
}

void EngineProfileView::Update() {
}

void EngineProfileView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(name_.c_str(), &isActive_, ImGuiWindowFlags_NoDocking);
	ImGui::Text("Engine Profile");
	ImGui::Separator();
	ImGui::Text("FPS: %.1f", QFE::EngineGlobalValue::fps);
	ImGui::Text("DeltaTime: %.3f", QFE::EngineGlobalValue::deltaTime);
	ImGui::End();
}
