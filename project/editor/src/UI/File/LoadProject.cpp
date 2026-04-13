#include "editor/include/UI/File/LoadProject.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

void QFE::LoadProject::Initialize() {
	currentProject_ = "";
	ignoreProjectList_ = { "Config","ShaderReflection" }; // ロードしないプロジェクトのリストを設定

	SetName("Load Project");
	isActive_ = false;
	LoadProjectDirectory();
	selected_ = 0;
}

void QFE::LoadProject::Update() {
	// プロジェクトのディレクトリを再取得するための更新処理
}

void QFE::LoadProject::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoDocking);
	if (ImGui::Button("LoadFiles")) {
		LoadProjectDirectory();
	}
	ImGui::Separator();
	if (!projectList_.empty()) {
		// ドロップダウンリストを表示
		std::vector<const char*> items;
		for (const auto& s : projectList_) items.push_back(s.c_str());
		if (ImGui::Combo("Project List", &selected_, items.data(), static_cast<int>(items.size()))) {
			currentProject_ = projectList_[selected_];
		}
		if (ImGui::Button("Load")) {
			SceneManager::GetInstance()->ResetProject(currentProject_);
			isActive_ = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isActive_ = false;
		}
	}

	ImGui::End();
}

void QFE::LoadProject::Run() {
	isActive_ = true;
}

void QFE::LoadProject::LoadProjectDirectory() {
	projectList_.clear();
	std::string resourceDir = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetRootDirectory();
	projectList_ = QFE::FILE::GetDirectoriesInDirectory(resourceDir);
	if (!projectList_.empty()) {
		currentProject_ = projectList_[0];

		// ロードしないプロジェクトをリストから削除
		for (const auto& ignoreProject : ignoreProjectList_) {
			projectList_.erase(std::remove(projectList_.begin(), projectList_.end(), ignoreProject), projectList_.end());
		}
	}
}
