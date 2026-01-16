/**
 * @file ConsoleView.cpp
 * @brief エンジンのデバッグログを表示するパネルの実装
 */

#include "editor/include/UI/View/ConsoleView.h"

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

/**
 * @brief コンストラクタ
 */
ConsoleView::ConsoleView() {
	name_ = "Console View";
	isActive_ = true;
#ifdef _DEBUG
	logLevel_ = LogLevel::EditorInfo;
#endif // _DEBUG
}

/**
 * @brief 初期化処理
 */
void ConsoleView::Initialize() {
}

/**
 * @brief 更新処理
 */
void ConsoleView::Update() {
}

/**
 * @brief 描画処理
 */
void ConsoleView::Draw() {
#ifdef _DEBUG
	if (!isActive_) {
		return;
	}
	ImGui::Begin(name_.c_str(), &isActive_); // Removed &isActive_ from the instruction, but keeping it as the instruction's snippet was partial and this is a common pattern.
	// フォーカス判定
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
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		MyDebugLog::GetInstance()->DebugLogClear();
	}
	ImGui::Separator();
	
	ImVec2 logAreaSize = ImGui::GetContentRegionAvail();
	ImGui::BeginChild("LogArea", logAreaSize, false, ImGuiWindowFlags_HorizontalScrollbar);

	auto& logs = MyDebugLog::GetInstance()->editorLog_;
	if (MyDebugLog::GetInstance()->errorLog_.size() > 0) {
		logLevel_ = LogLevel::Error;
	}

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

		// 右クリックメニュー追加
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Copy")) {
				ImGui::SetClipboardText(it->c_str());
			}
			ImGui::EndPopup();
		}
	}
	ImGui::EndChild();
	ImGui::End();

#endif // _DEBUG
}
