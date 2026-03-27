/**
 * @file LoadScene.cpp
 * @brief シーンロード用ブラウザの実装
 */

#include "editor/include/UI/File/LoadScene.h"
#include "scene/SceneManager.h"
using namespace QFE;
void LoadScene::Initialize() {
	currentScene_ = "";
	SetName("Load Scene");
	isActive_ = false;
	sceneList_.clear();
	sceneList_ = QFE::FILE::GetFilesInDirectory("Resources/Scenes", ".json");
	if (!sceneList_.empty()) {
		currentScene_ = sceneList_[0];
	}
	selected_ = 0;
}

void LoadScene::Update() {
}

void LoadScene::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoDocking);
	if (ImGui::Button("LoadFiles")) {
		sceneList_ = QFE::FILE::GetFilesInDirectory("Resources/Scenes", ".json");
	}
	ImGui::Separator();
	if (!sceneList_.empty()) {
		// ドロップダウンリストを表示
		std::vector<const char*> items;
		for (const auto& s : sceneList_) items.push_back(s.c_str());
		if (ImGui::Combo("Scene List", &selected_, items.data(), static_cast<int>(items.size()))) {
			currentScene_ = sceneList_[selected_];
		}
		if (ImGui::Button("Load")) {
			SceneManager::GetInstance()->LoadScene(currentScene_);
			isActive_ = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isActive_ = false;
		}
	}

	ImGui::End();
}

void LoadScene::Run() {
	isActive_ = true;
}
