/**
 * @file ConsoleView.cpp
 * @brief エンジンのデバッグログを表示するパネルの実装
 */

#include "editor/include/UI/View/ConsoleView.h"
#include "engine/include/core/Bridge/EngineBridgeProvider.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
using namespace QFE;
/**
 * @brief コンストラクタ
 */
ConsoleView::ConsoleView() {
	SetName("Console");
	isActive_ = true;
#ifdef QFE_OPTIMIZE_OFF
	logLevel_ = LogLevel::EditorInfo;
#endif // _DEBUG
}

/**
 * @brief 初期化処理
 */
void ConsoleView::Initialize() {
	logDrawFunctions_[LogKind::ByLevel] = std::bind(&ConsoleView::DrawLogsByLevel, this);
	logDrawFunctions_[LogKind::ByLocation] = std::bind(&ConsoleView::DrawLogsByLocation, this);
	currentLogKind_ = LogKind::ByLevel;
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
#ifdef QFE_OPTIMIZE_OFF
	if (!isActive_) {
		return;
	}
	ImGui::Begin(GetName().c_str(), &isActive_);

	// ログの表示方法切り替え用のコンボボックス
	ImGui::Text("Display Mode:");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##displaymode",
		currentLogKind_ == LogKind::ByLevel ? "By Level" :
		currentLogKind_ == LogKind::ByLocation ? "By Location" : "Unknown")) {
		if (ImGui::Selectable("By Level", currentLogKind_ == LogKind::ByLevel)) {
			currentLogKind_ = LogKind::ByLevel;
		}
		if (ImGui::Selectable("By Location", currentLogKind_ == LogKind::ByLocation)) {
			currentLogKind_ = LogKind::ByLocation;
		}
		ImGui::EndCombo();
	}

	// 現在の表示方法に応じた描画関数を呼び出す
	logDrawFunctions_[currentLogKind_]();
	ImGui::End();

#endif // _DEBUG
}

void QFE::ConsoleView::DrawLogsByLevel()
{
#ifdef QFE_OPTIMIZE_OFF
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
		IEngineBridge* engineBridge = QFE::BRIDGE::GetBridge();
		engineBridge->ClearRuntimeDebugLogs();
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
#endif // _DEBUG
}

void QFE::ConsoleView::DrawLogsByLocation()
{
	#ifdef QFE_OPTIMIZE_OFF
	ImGui::Text("Logs by Location:");
	ImGui::Separator();
	for (const auto& [className, funcMap] : MyDebugLog::GetInstance()->locationLogMap_) {
		if (ImGui::TreeNode(className.c_str())) {
			for (const auto& [funcName, logs] : funcMap) {
				if (ImGui::TreeNode(funcName.c_str())) {
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
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}
#endif // _DEBUG
}
