#include "CreateNewScene.h"
#include "Scene/SceneManager.h"

void CreateNewScene::Initialize() {
	name_ = "New Scene";
	isActive_ = false;
}

void CreateNewScene::Update() {
}

void CreateNewScene::Draw() {
	if (!isActive_) {
		return;
	}
	ImGui::OpenPopup(name_.c_str());
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);
	if (ImGui::BeginPopupModal(name_.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Create a new scene?\n");
		ImGui::Separator();
		if (ImGui::Button("Create", ImVec2(120, 0))) {
			SceneManager::GetInstance()->ResetScene();
			isActive_ = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			isActive_ = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void CreateNewScene::Run() {
	isActive_ = true;
}
