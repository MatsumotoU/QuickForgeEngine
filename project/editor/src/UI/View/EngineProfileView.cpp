/**
 * @file EngineProfileView.cpp
 * @brief エンジンのパフォーマンス統計（FPS等）を表示するパネルの実装
 */

#include "editor/include/UI/View/EngineProfileView.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/BuildInfo.h"
using namespace QFE;
EngineProfileView::EngineProfileView() {
	isActive_ = false;
	SetName("Engine Profile View");
}

void EngineProfileView::Initialize() {
}

void EngineProfileView::Update() {
}

void EngineProfileView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoDocking);
	ImGui::Text("Engine Profile");
	ImGui::Separator();
	
	ImGui::Text("FPS: %.1f", QFE::EngineGlobalValue::fps);
	ImGui::Text("DeltaTime: %.3f", QFE::EngineGlobalValue::deltaTime);
	ImGui::End();
}
