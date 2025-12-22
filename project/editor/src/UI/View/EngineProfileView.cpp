#include "editor/include/UI/View/EngineProfileView.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/BuildInfo.h"

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
	if (ImGui::CollapsingHeader("Build Info")) {
		ImGui::Text("Build Commit: %s", BUILD_COMMIT);
		ImGui::Text("Build Branch: %s", BUILD_BRANCH);
		ImGui::Text("Build Date: %s", BUILD_DATE);
		ImGui::Text("Build Time: %s", BUILD_TIME);
	}
	
	ImGui::Text("FPS: %.1f", QFE::EngineGlobalValue::fps);
	ImGui::Text("DeltaTime: %.3f", QFE::EngineGlobalValue::deltaTime);
	ImGui::End();
}
