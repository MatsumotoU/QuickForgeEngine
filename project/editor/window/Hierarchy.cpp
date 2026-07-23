#include "Hierarchy.h"
#include "design-patterns/EntityManager.h"
#include "components/AllComponent.h"
#include "EngineDefines.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"

#include <imgui/imgui.h>

#include "framework/window/WindowsWindowFrameWork.h"

QFE::EDITOR::Hierarchy::Hierarchy(EntityManager* entityManager)
	: entityManager_(entityManager), isActive_(true) {}

void QFE::EDITOR::Hierarchy::Initialize() {
	isActive_ = true;
}

void QFE::EDITOR::Hierarchy::Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_);
	ImVec2 windowSize = ImGui::GetWindowSize();
	isFocus_ = ImGui::IsWindowFocused();

	// エンティティマネージャーが null の場合は、エラーメッセージを表示して終了する
	if(entityManager_ == nullptr) {
		ImGui::Text("EntityManager is null.");
		return;
	}

	std::vector<uint32_t> entityIds =entityManager_->GetActiveEntityIds();
	ImGui::Text("Active Entities: %zu", entityIds.size());

	// EntityManagerからObjectInfoComponentを持つエンティティを取得して表示
	ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("EntityList", ImVec2(0, 0), child_flags)) {

		entityManager_->GetComponentStrage<QFE::SCENE::ObjectInfoComponent>().Each([&](
			uint32_t entityId, QFE::SCENE::ObjectInfoComponent& objectInfoComp) {

				bool currentSelected = hierarchySelectedEntities_.contains(entityId);

				if (ImGui::Selectable((objectInfoComp.name + "##" + std::to_string(entityId)).c_str(), currentSelected)) {
					if (ImGui::GetIO().KeyCtrl) {
						// Ctrl押し：トグル
						if (currentSelected) {
							hierarchySelectedEntities_.erase(entityId);
						} else {
							hierarchySelectedEntities_.insert(entityId);
						}
					} else {
						// Ctrlなし：単一選択
						hierarchySelectedEntities_.clear();
						hierarchySelectedEntities_.insert(entityId);
					}
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					cameraFocusRequest_ = entityId;
				}
			});

		// このChildウィンドウがクリックされた、かつ、どのSelectable（アイテム）もホバーされていない場合
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
			hierarchySelectedEntities_.clear();
		}
	}
	ImGui::EndChild();

	ImGui::Text("Selected Entities: %zu", hierarchySelectedEntities_.size());
	for(uint32_t entityId : hierarchySelectedEntities_) {
		ImGui::Text("Entity ID: %u", entityId);
	}

	// 右クリックでコンテキストメニューを表示する
	// IsWindowHovered() でこのウィンドウ内を指しているか、かつ右クリックが押されたか
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		ImGui::OpenPopup("EntityContextMenu");
	}

	// ポップアップの描画処理（BeginPopupContextWindow ではなく BeginPopup を使う）
	if (ImGui::BeginPopup("EntityContextMenu")) {
		// 新しいエンティティを作成する
		if (ImGui::MenuItem("CreateEmptyEntity")) {
			commandList.AddCommand(std::make_unique<CreateEntityCommand>("NewEntity", QFE::MATH::Vector3(0, 0, 0), entityManager_));
		}

		// 選択されたエンティティがある場合のみ、コピーと削除のメニューを表示する
		if (!hierarchySelectedEntities_.empty()) {
			// 選択されたエンティティをコピーする
			if (ImGui::MenuItem("CopyEntity")) {
				for (uint32_t entityId : hierarchySelectedEntities_) {
					commandList.AddCommand(std::make_unique<CopyEntityCommand>(entityId, entityManager_));
				}
			}
			// 選択されたエンティティを削除する
			if (ImGui::MenuItem("DeleteEntity")) {
				for (uint32_t entityId : hierarchySelectedEntities_) {
					commandList.AddCommand(std::make_unique<DeleteEntityCommand>(entityId, entityManager_));
				}
			}
		}

		ImGui::EndPopup();
	}

	ImGui::End();

	// ヒエラルキーで選択されたエンティティを、外部のselectedEntitiesセットに追加する
	for (uint32_t entityId : hierarchySelectedEntities_) {
		selectedEntities.insert(entityId);
	}
}

std::string QFE::EDITOR::Hierarchy::GetWindowName() {
	return "Hierarchy";
}

bool QFE::EDITOR::Hierarchy::GetIsActive() {
	return isActive_;
}

bool QFE::EDITOR::Hierarchy::SetIsActive(bool isActive) {
	isActive_ = isActive;
	return isActive_;
}

bool QFE::EDITOR::Hierarchy::GetIsFocus() {
	return isFocus_;
}

std::optional<uint32_t> QFE::EDITOR::Hierarchy::ConsumeCameraFocusRequest() {
	std::optional<uint32_t> request = cameraFocusRequest_;
	cameraFocusRequest_.reset();
	return request;
}
