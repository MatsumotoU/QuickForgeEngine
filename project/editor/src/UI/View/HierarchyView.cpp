/**
 * @file HierarchyView.cpp
 * @brief シーン内のエンティティ階層を表示・操作するパネルの実装
 */

#include "editor/include/UI/View/HierarchyView.h"

#include "engine/include/core/Bridge/EngineBridgeProvider.h"
#include "engine/include/core/Bridge/IEngineBridge.h"

#ifdef QFE_OPTIMIZE_OFF
#include "utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

uint32_t HierarchyView::selectedEntityId_ = 0;

HierarchyView::HierarchyView() {
	isActive_ = true;
	name_ = "Hierarchy View";
#ifdef QFE_OPTIMIZE_OFF
	particleCount_ = 1;
#endif // _DEBUG
}

void HierarchyView::Initialize() {
#ifdef QFE_OPTIMIZE_OFF
	QFE::IEngineBridge* bridge = QFE::EngineBridgeProvider::Get();
	if (bridge) {
		modelDropDownFileList_.LoadFileList(bridge->GetModelDirectoryPath(), ".obj");
		spriteDropDownFileList_.LoadFileList(bridge->GetImageDirectoryPath(), ".png");
		entityDropDownFileList_.LoadFileList(bridge->GetEntityTemplateDirectoryPath(), ".json");
	}
#endif // _DEBUG
}

void HierarchyView::Update() {

}

/** @brief 描画 */
void HierarchyView::Draw() {
#ifdef QFE_OPTIMIZE_OFF
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
#ifdef QFE_OPTIMIZE_OFF
	QFE::IEngineBridge* bridge = QFE::EngineBridgeProvider::Get();
	if (!bridge) return;

	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Reload FileList")) {
			Initialize();
		}

		if (ImGui::BeginMenu("Add")) {
			if (ImGui::MenuItem("Empty Entity")) {
				bridge->AddEmptyEntity();
			}

			if (ImGui::BeginMenu("Entity")) {
				entityDropDownFileList_.DrawMenuItem();
				std::string selectedEntityFileName_;
				if (entityDropDownFileList_.GetSelectedFileName(selectedEntityFileName_)) {
					bridge->AddEntityFromFile(selectedEntityFileName_);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Model")) {
				modelDropDownFileList_.DrawMenuItem();
				std::string selectedModelFileName_;
				if (modelDropDownFileList_.GetSelectedFileName(selectedModelFileName_)) {
					bridge->AddModelEntity(selectedModelFileName_);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Sprite")) {
				spriteDropDownFileList_.DrawMenuItem();
				std::string selectedSpriteFileName_;
				if (spriteDropDownFileList_.GetSelectedFileName(selectedSpriteFileName_)) {
					bridge->AddSpriteEntity(selectedSpriteFileName_);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Particle Emitter")) {
				ImGui::DragInt("Particle Count", &particleCount_);
				ImGui::Separator();
				modelDropDownFileList_.DrawMenuItem();
				std::string selectedModelFileName_;
				if (modelDropDownFileList_.GetSelectedFileName(selectedModelFileName_)) {
					bridge->AddParticleEmitterEntity(selectedModelFileName_, static_cast<uint32_t>(particleCount_));
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Camera")) {
#ifdef _DEBUG
				bridge->AddCameraEntity();
#endif // _DEBUG
			}

			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
#endif // _DEBUG
}

void HierarchyView::DrawEntityList() {
#ifdef QFE_OPTIMIZE_OFF
	QFE::IEngineBridge* bridge = QFE::EngineBridgeProvider::Get();
	if (!bridge) return;

	auto entityIds = bridge->GetAllEntityIds();
	for (uint32_t id : entityIds) {
		bool isSelected = (selectedEntityId_ == id);
		std::string name = bridge->GetEntityName(id);
		std::string label = name + "##" + std::to_string(id);

		// デバッグカメラは表示しない
#ifdef QFE_OPTIMIZE_OFF
		if (id == bridge->GetDebugCameraEntityId()) {
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
					bridge->ParentChild(id, draggedId);
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
				bridge->CopyEntity(id);
			}
			if (ImGui::MenuItem("Save")) {
				// 保存処理
				bridge->SaveEntity(id, name);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Delete")) {
				bridge->DeleteEntity(id);

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
