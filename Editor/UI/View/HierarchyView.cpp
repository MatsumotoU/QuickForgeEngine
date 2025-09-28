#include "HierarchyView.h"
#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Scene/SceneManager.h"
#include "Scene/Data/SceneObjectData.h"

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
	auto entityIds = assetManager->GetEntityManager()->GetActiveEntityIds();
    for (uint32_t id : entityIds) {
        bool isSelected = (selectedEntityId_ == id);
		std::string& name = assetManager->GetEntityManager()->GetComponent<SceneObjectData>(id).name;
        std::string label = name + "##" + std::to_string(id);

        // Selectable（左クリックで選択）
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            selectedEntityId_ = id;
        }

        // 右クリックでコンテキストメニュー
        if (ImGui::BeginPopupContextItem(label.c_str())) {
            if (ImGui::MenuItem("Delete")) {
                // 削除処理
                AssetManager::GetInstance()->GetEntityManager()->RemoveEntity(id);

                // 選択中だったら選択解除
                if (selectedEntityId_ == id) {
                    selectedEntityId_ = 0;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}