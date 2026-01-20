/**
 * @file HierarchyView.cpp
 * @brief シーン内のエンティティ階層を表示・操作するパネルの実装
 */

#include "editor/include/UI/View/HierarchyView.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#ifdef _DEBUG
#include "utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

uint32_t HierarchyView::selectedEntityId_ = 0;

HierarchyView::HierarchyView() {
	isActive_ = true;
	name_ = "Hierarchy View";
#ifdef _DEBUG
	particleCount_ = 1;
#endif // _DEBUG
}

void HierarchyView::Initialize() {
#ifdef _DEBUG
	modelDropDownFileList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Model"), ".obj");
	spriteDropDownFileList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Image"), ".png");
	entityDropDownFileList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Entities"), ".json");
#endif // _DEBUG
}

void HierarchyView::Update() {

}

/** @brief 描画 */
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
		if (ImGui::MenuItem("Reload FileList")) {
			Initialize();
		}

		if (ImGui::BeginMenu("Add")) {
			if (ImGui::MenuItem("Empty Entity")) {
				SceneManager::GetInstance()->AddEmptyObject();
			}

			if (ImGui::BeginMenu("Entity")) {
				entityDropDownFileList_.DrawMenuItem();
				std::string selectedEntityFileName_;
				if (entityDropDownFileList_.GetSelectedFileName(selectedEntityFileName_)) {
					SceneManager::GetInstance()->AddEntity(selectedEntityFileName_);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Model")) {
				modelDropDownFileList_.DrawMenuItem();
				std::string selectedModelFileName_;
				if (modelDropDownFileList_.GetSelectedFileName(selectedModelFileName_)) {
					SceneManager::GetInstance()->AddModel(selectedModelFileName_);
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

			if (ImGui::BeginMenu("Particle Emitter")) {
				ImGui::DragInt("Particle Count", &particleCount_);
				ImGui::Separator();
				modelDropDownFileList_.DrawMenuItem();
				std::string selectedModelFileName_;
				if (modelDropDownFileList_.GetSelectedFileName(selectedModelFileName_)) {
					SceneManager::GetInstance()->AddParticleEmitter(selectedModelFileName_, static_cast<uint32_t> (particleCount_));
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Camera")) {
#ifdef _DEBUG
				DebugLog("Can not Add Camera");
#endif // _DEBUG
			}

			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
#endif // _DEBUG
}

void HierarchyView::DrawEntityList() {
#ifdef _DEBUG
	AssetManager* assetManager = AssetManager::GetInstance();
	auto entityIds = assetManager->GetEntityManager()->GetActiveEntityIds();
	for (uint32_t id : entityIds) {
		bool isSelected = (selectedEntityId_ == id);
		SceneObjectData& data = assetManager->GetEntityManager()->GetComponent<SceneObjectData>(id);
		std::string& name = data.name;
		std::string label = name + "##" + std::to_string(id);

		// デバッグカメラは表示しない
#ifdef _DEBUG
		if (id == CameraManager::GetInstance()->GetCamera(0).GetBindEntityId()) {
			continue;
		}
#endif // _DEBUG

		// ドラッグソース
		ImGui::PushID(id);
		if (ImGui::Selectable(label.c_str(), isSelected)) {
			selectedEntityId_ = id;
		}

		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("ENTITY_ID", &id, sizeof(id));
			ImGui::Text("%s", name.c_str());
			ImGui::EndDragDropSource();
		}

		// ドロップターゲット
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ID")) {
				uint32_t draggedId = *(const uint32_t*)payload->Data;
				if (draggedId != id) {
					// 親子関係を設定
					SceneManager::GetInstance()->ParentChild(id, draggedId);
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopID();

		// 右クリックでコンテキストメニュー
		if (ImGui::BeginPopupContextItem(label.c_str())) {
			if (ImGui::MenuItem("Rename")) {
				ImGui::OpenPopup("Rename Entity");
			}
			if (ImGui::MenuItem("Copy")) {
				SceneManager::GetInstance()->CopyEntity(id);
			}
			if (ImGui::MenuItem("Save")) {
				// 保存処理
				SceneManager::GetInstance()->SaveEntity(id, name);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Delete")) {
				SceneManager::GetInstance()->DeleteEntity(id);

				// 選んでいるEntityが消された場合、選択解除
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
