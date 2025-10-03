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
#ifdef _DEBUG
    modelDropDownFileList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Model"), ".obj");
	spriteDropDownFileList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Image"), ".png");
#endif // _DEBUG
}

void HierarchyView::Update() {

}

void HierarchyView::Draw() {
#ifdef _DEBUG
    if (!isActive_) {
        return;
    }

    ImGui::Begin(name_.c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    // 右クリックでコンテキストメニュー
    DrawPopupContextWindow();
    // Entity一覧表示
    DrawEntityList();

    ImGui::End();
#endif // _DEBUG
}

void HierarchyView::DrawPopupContextWindow() {
#ifdef _DEBUG
    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Empty Entity")) {
                SceneManager::GetInstance()->AddEpmtyObject();
            }

            if (ImGui::BeginMenu("Model")) {
                modelDropDownFileList_.DrawMenuItem();
                std::string selectedModelFileName_;
                if (modelDropDownFileList_.GetSelectedFileName(selectedModelFileName_)) {
                    SceneManager::GetInstance()->LoadModel(selectedModelFileName_);
                }
                ImGui::EndMenu();
            }
            
			if (ImGui::BeginMenu("Sprite")) {
				spriteDropDownFileList_.DrawMenuItem();
				std::string selectedSpriteFileName_;
				if (spriteDropDownFileList_.GetSelectedFileName(selectedSpriteFileName_)) {
					SceneManager::GetInstance()->AddSprite(selectedSpriteFileName_);
				}
				ImGui::EndMenu();
			}

            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
#endif // _DEBUG
}

void HierarchyView::DrawEntityList() {
#ifdef _DEBUG
    // TODO: 名前をつけられるようにする
    AssetManager* assetManager = AssetManager::GetInstance();
    auto entityIds = assetManager->GetEntityManager()->GetActiveEntityIds();
    for (uint32_t id : entityIds) {
        bool isSelected = (selectedEntityId_ == id);
        std::string& name = assetManager->GetEntityManager()->GetComponent<SceneObjectData>(id).name;
        std::string label = name + "##" + std::to_string(id);

        if (isSelected) {
            // 選択中はInputTextで名前編集
            char buf[128];
            strncpy_s(buf, sizeof(buf), name.c_str(), _TRUNCATE);
            buf[sizeof(buf) - 1] = '\0';
            ImGui::PushID(id);
            if (ImGui::InputText("##edit", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                name = buf;
            }
            ImGui::PopID();
            // 選択解除用
            if (ImGui::IsItemDeactivatedAfterEdit() && !ImGui::IsItemActive()) {
                selectedEntityId_ = 0;
            }
        } else {
            // 通常はSelectable
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                selectedEntityId_ = id;
            }
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
#endif // _DEBUG
}