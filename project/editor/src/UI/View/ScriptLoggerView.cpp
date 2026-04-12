/**
 * @file ScriptLoggerView.cpp
 * @brief 各エンティティにアタッチされたスクリプトからの出力を表示するパネルの実装
 */

#include "editor/include/UI/View/ScriptLoggerView.h"

#include "editor/include/UI/View/HierarchyView.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
using namespace QFE;

ScriptLoggerView::ScriptLoggerView() {
	isActive_ = true;
	SetName("Script Logger");
	selectedEntityId_ = 0;
}

void ScriptLoggerView::Initialize() {
	isActive_ = true;
}

void ScriptLoggerView::Update() {
	selectedEntityId_ = HierarchyView::selectedEntityId_;
}

void ScriptLoggerView::Draw() {
	if (!isActive_) {
		return;
	}
	ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	ImGui::Text("Selected Entity ID: %d", selectedEntityId_);
	if (ImGui::Button("Clear Logs")) {
#ifdef QFE_OPTIMIZE_OFF
		MyDebugLog::GetInstance()->scriptLogs_.clear();
#endif // _DEBUG
	}
	
#ifdef QFE_OPTIMIZE_OFF
	uint32_t id = selectedEntityId_;
	auto it = MyDebugLog::GetInstance()->scriptLogs_.find(id);
	if (it != MyDebugLog::GetInstance()->scriptLogs_.end()) {
		for (const auto& [scriptName, logs] : it->second) {
			ImGui::Separator();
			ImGui::Text("Script: %s", scriptName.c_str());
			ImGui::BeginChild(("LogChild_" + scriptName).c_str(), ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
			for (auto itL = logs.rbegin(); itL != logs.rend(); ++itL) {
				ImGui::TextWrapped("%s", itL->c_str());
			}
			ImGui::EndChild();
			if (logs.size() >= 100) {
				ImGui::Text("Only the latest 100 logs are displayed");
			}
		}
	} else {
		ImGui::Separator();
		ImGui::Text("No script logs for this entity.");
	}
#endif // _DEBUG
	ImGui::End();
}
