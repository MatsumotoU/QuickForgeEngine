#include "Hierarchy.h"
#include "design-patterns/EntityManager.h"
#include "components/AllComponent.h"
#include "EngineDefines.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"

#include <imgui/imgui.h>

QFE::EDITOR::Hierarchy::Hierarchy(EntityManager* entityManager)
	: entityManager_(entityManager), isActive_(true) {}

void QFE::EDITOR::Hierarchy::Initialize() {
	isActive_ = true;
}

void QFE::EDITOR::Hierarchy::Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_);
	ImVec2 windowSize = ImGui::GetWindowSize();

	// エンティティマネージャーが null の場合は、エラーメッセージを表示して終了する
	if(entityManager_ == nullptr) {
		ImGui::Text("EntityManager is null.");
		return;
	}

	bool isSelected = false;

	std::vector<uint32_t> entityIds =entityManager_->GetActiveEntityIds();
	ImGui::Text("Active Entities: %zu", entityIds.size());

	// EntityManagerからObjectInfoComponentを持つエンティティを取得して表示
	ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY;
	std::vector<uint32_t> hierarchySelectedEntities;// ヒエラルキーで選択されたエンティティを一時的に保持するためのベクター
	if (ImGui::BeginChild("EntityList", ImVec2(0, 0), child_flags)) {
		entityManager_->GetComponentStrage<QFE::SCENE::ObjectInfoComponent>().Each([&](
			uint32_t entityId, QFE::SCENE::ObjectInfoComponent& objectInfoComp) {
				bool currentSelected = selectedEntities.contains(entityId);
				if (ImGui::Selectable((objectInfoComp.name + "##" + std::to_string(entityId)).c_str(), currentSelected)) {
					hierarchySelectedEntities.push_back(entityId);
				}
			});
	}
	ImGui::EndChild();

	ImGui::Text("Selected Entities: %zu", hierarchySelectedEntities.size());
	for(uint32_t& entityId : hierarchySelectedEntities) {
		ImGui::Text("Entity ID: %u", entityId);
	}

	// 右クリックでコンテキストメニューを表示する
	if (ImGui::BeginPopupContextWindow("EntityContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
		// 新しいエンティティを作成する
		if (ImGui::MenuItem("CreateEmptyEntity")) {
			commandList.AddCommand(std::make_unique<CreateEntityCommand>("NewEntity", QFE::MATH::Vector3(0, 0, 0), entityManager_));
		}
		// 選択されたエンティティがある場合のみ、コピーと削除のメニューを表示する
		if(!hierarchySelectedEntities.empty()) {
			// 選択されたエンティティをコピーする
			if (ImGui::MenuItem("CopyEntity")) {
				for (uint32_t entityId : hierarchySelectedEntities) {
					commandList.AddCommand(std::make_unique<CopyEntityCommand>(entityId, entityManager_));
				}
			}
			// 選択されたエンティティを削除する
			if (ImGui::MenuItem("DeleteEntity")) {
				for (uint32_t entityId : hierarchySelectedEntities) {
					commandList.AddCommand(std::make_unique<DeleteEntityCommand>(entityId, entityManager_));
				}
			}
		}
		
		ImGui::EndPopup();
	}

	ImGui::End();

	// ヒエラルキーで選択されたエンティティを、外部のselectedEntitiesセットに反映させる
	for(uint32_t& entityId : hierarchySelectedEntities) {
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
