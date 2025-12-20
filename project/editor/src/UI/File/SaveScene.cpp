#include "editor/include/UI/File/SaveScene.h"
#include "scene/SceneManager.h"

void SaveScene::Initialize() {
	name_ = "SaveScene";
	isActive_ = false;
	sceneName_ = "NewScene";
}

void SaveScene::Update() {
}

void SaveScene::Draw() {
	if (!isActive_) {
		return;
	}
	ImGui::Begin("Save Scene", &isActive_, ImGuiWindowFlags_NoDocking);
	
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "%s", sceneName_.c_str());

		if (ImGui::InputText("Scene Name", buffer, sizeof(buffer))) {
			sceneName_ = buffer;
		}
		if (ImGui::Button("Save")) {
			SceneManager::GetInstance()->SaveScene(sceneName_);
			isActive_ = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			isActive_ = false;
		}
	
	ImGui::End();
}

void SaveScene::Run() {
	isActive_ = true;
	sceneName_ = SceneManager::GetInstance()->GetCurrentSceneName();
}
