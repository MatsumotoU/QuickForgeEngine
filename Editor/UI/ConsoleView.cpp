#include "ConsoleView.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/ImGui/FrameController/ImGuiFlameController.h"
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

ConsoleView::ConsoleView() {
	name_ = "Console View";
	isActive_ = true;
	logLevel_ = LogLevel::EditorInfo;
}

void ConsoleView::Initialize() {
}

void ConsoleView::Update() {
}

void ConsoleView::Draw() {
	if (!isActive_) {
		return;
	}
	ImGui::Begin(name_.c_str(), &isActive_);
	ImGui::Text("Log Level:");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##loglevel",
		logLevel_ == LogLevel::EngineInfo ? "Engine Info" :
		logLevel_ == LogLevel::EditorInfo ? "Editor Info" :
		logLevel_ == LogLevel::Warning ? "Warning" :
		logLevel_ == LogLevel::Error ? "Error" : "All")) {
		if (ImGui::Selectable("Engine Info", logLevel_ == LogLevel::EngineInfo)) {
			logLevel_ = LogLevel::EngineInfo;
		}
		if (ImGui::Selectable("Editor Info", logLevel_ == LogLevel::EditorInfo)) {
			logLevel_ = LogLevel::EditorInfo;
		}
		if (ImGui::Selectable("Warning", logLevel_ == LogLevel::Warning)) {
			logLevel_ = LogLevel::Warning;
		}
		if (ImGui::Selectable("Error", logLevel_ == LogLevel::Error)) {
			logLevel_ = LogLevel::Error;
		}
		ImGui::EndCombo();
	}
	ImGui::Separator();
	
	ImVec2 logAreaSize = ImGui::GetContentRegionAvail();
	ImGui::BeginChild("LogArea", logAreaSize, false, ImGuiWindowFlags_HorizontalScrollbar);

	auto& logs = MyDebugLog::GetInstance()->editorLog_;
	switch (logLevel_)
	{
	case LogLevel::EngineInfo:
		logs = MyDebugLog::GetInstance()->engineLog_;
		break;
	case LogLevel::EditorInfo:
		logs = MyDebugLog::GetInstance()->editorLog_;
		break;
	case LogLevel::Warning:
		logs = MyDebugLog::GetInstance()->warningLog_;
		break;
	case LogLevel::Error:
		logs = MyDebugLog::GetInstance()->errorLog_;
		break;
	default:
		break;
	}

	int logIndex = 0;
	for (auto it = logs.rbegin(); it != logs.rend(); ++it, ++logIndex) {
		std::string label = *it + "##log" + std::to_string(logIndex);
		ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);
	}
	ImGui::EndChild();
	ImGui::End();
}
