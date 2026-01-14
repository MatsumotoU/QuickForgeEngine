#include "editor/include/UI/View/InputLogView.h"
#include "Engine/include/input/InputInterface.h"
#include "Engine/include/utility/String/DirectInputToString.h"

InputLogView::InputLogView()
{
	isActive_ = false;
	name_ = "Input Log View";
}

void InputLogView::Initialize()
{
}

void InputLogView::Update()
{
}

void InputLogView::Draw()
{
	if (!isActive_) { return; }
	InputInterface* input = InputInterface::GetInstance();

	ImGui::Begin("Input Log View", &isActive_);
	// キーの録画・停止のコントロール
	if (input->GetInputLogger().IsRecording()) {
		if (ImGui::Button("Stop Recording"))
		{
			input->GetInputLogger().StopRecording();
		}
	}
	else{
		if (ImGui::Button("Start Recording"))
		{
			input->GetInputLogger().StartRecording();
		}
	}
	ImGui::Spacing();

	// 録画された入力ログの表示
	if (ImGui::CollapsingHeader("Input Log Data", ImGuiTreeNodeFlags_DefaultOpen)) {
		const auto& logDataList = input->GetInputLogger().GetInputLogDataList();
		ImGui::BeginChild("InputLogViewChild", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		// テーブルで整列表示
		if (ImGui::BeginTable("InputLogTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("StartFrame");
			ImGui::TableSetupColumn("EndFrame");
			ImGui::TableSetupColumn("KeyCode");
			ImGui::TableSetupColumn("PressedFrame");
			ImGui::TableHeadersRow();

			for (const auto& logData : logDataList) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d F", logData.startFrame_);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%d F", logData.endFrame_);
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", DirectInputToString(logData.pressedKeyCode_).c_str());
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%d F", logData.endFrame_ - logData.startFrame_);
			}
			ImGui::EndTable();
		}

		// スクロールを一番下に自動で移動
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
	}

	ImGui::End();
}

void InputLogView::Run()
{
}
