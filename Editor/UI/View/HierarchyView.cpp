#include "HierarchyView.h"
#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Scene/SceneManager.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

uint32_t HierarchyView::selectedEntityId_ = 0;

HierarchyView::HierarchyView() {
	isActive_ = true;
	name_ = "Hierarchy View";
}

void HierarchyView::Initialize() {
	modelDropDownFileList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Model"), ".obj");
}

void HierarchyView::Update() {

}

void HierarchyView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(name_.c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	// 右クリックでコンテキストメニュー
	DrawPopupContextWindow();
	// Entity一覧表示
	DrawEntityList();

	ImGui::End();
}

void HierarchyView::DrawPopupContextWindow() {
    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::BeginMenu("Model")) {
                modelDropDownFileList_.DrawMenuItem();
                std::string selectedModelFileName_;
                if (modelDropDownFileList_.GetSelectedFileName(selectedModelFileName_)) {
					SceneManager::GetInstance()->LoadModel(selectedModelFileName_);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

void HierarchyView::DrawEntityList() {
	// TODO: 名前をつけられるようにする
	AssetManager* assetManager = AssetManager::GetInstance();
	for (uint32_t i = 0; i < assetManager->GetEntityManager()->GetNextEntityId(); i++) {
		// 選択状態かどうか
		bool isSelected = (selectedEntityId_ == i);
		// クリック可能なリスト項目
		if (ImGui::Selectable(("GameObject " + std::to_string(i)).c_str(), isSelected)) {
			selectedEntityId_ = i;
		}
	}
}