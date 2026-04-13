#include "editor/include/UI/File/CreateNewProject.h"
#include "engine/include/scene/SceneManager.h"

void QFE::CreateNewProject::Initialize() {
	SetName("Create New Project");
	isActive_ = false;
	projectName_.clear();
}

void QFE::CreateNewProject::Update() {
	// ここは特に何もしない
}

void QFE::CreateNewProject::Draw() {
	if (!isActive_) {
		return;
	}
	ImGui::OpenPopup(GetName().c_str());
	ImGui::SetNextWindowSize(ImVec2(300, 130), ImGuiCond_Once);
	if (ImGui::BeginPopupModal(GetName().c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Create a new project?\n");

		// 入力バッファとして十分なサイズ(例: 64)まで確保。
		// すでに 64 以上の場合はそのまま。
		if (projectName_.size() < 64) {
			projectName_.resize(64);
		}

		// &projectName_[0] を渡すことで、内部バッファのポインタを安全に取得・更新
		// (C++14において data() は const char* を返す可能性があるため operator[] を使用)
		ImGui::InputText("Project Name", &projectName_[0], 64);

		ImGui::Separator();
		if (ImGui::Button("Create", ImVec2(120, 0))) {
			// ImGuiが書き込んだ終端文字 '\0' の位置を探して、文字列長を正しい長さに切り詰める
			size_t nullPos = projectName_.find('\0');
			if (nullPos != std::string::npos) {
				projectName_.resize(nullPos);
			}

			SceneManager::GetInstance()->ResetProject(projectName_);

			// 次回の表示のためクリア
			projectName_.clear();

			isActive_ = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			isActive_ = false;
			projectName_.clear(); // キャンセル時はクリア
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void QFE::CreateNewProject::Run() {
	isActive_ = true;
}