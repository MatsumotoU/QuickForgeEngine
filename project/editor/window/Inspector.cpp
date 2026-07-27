#include "Inspector.h"
#include "design-patterns/EntityManager.h"
#include "inspector/ImGuiArchive.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"

QFE::EDITOR::Inspector::Inspector(QFE::EntityManager* entityManager) :
	entityManager_(entityManager), isActive_(true), isFocus_(false) {
}

void QFE::EDITOR::Inspector::Initialize() {
}

void QFE::EDITOR::Inspector::Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_);
	isFocus_ = ImGui::IsWindowFocused();

	if (selectedEntities.empty()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	uint32_t selectedEntityId = *selectedEntities.begin();
	const float footerHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f;
	if (ImGui::BeginChild("Components", ImVec2(0, -footerHeight), ImGuiChildFlags_Border)) {
		DrawComponents(selectedEntityId);
	}
	ImGui::EndChild();

	AddComponentUI(selectedEntityId, commandList);

	ImGui::End();
}

std::string QFE::EDITOR::Inspector::GetWindowName() {
	return "Inspector";
}

bool QFE::EDITOR::Inspector::GetIsActive() {
	return isActive_;
}

bool QFE::EDITOR::Inspector::SetIsActive(bool isActive) {
	isActive_ = isActive;
	return isActive_;
}

bool QFE::EDITOR::Inspector::GetIsFocus() {
	return isFocus_;
}

void QFE::EDITOR::Inspector::DrawComponents(uint32_t entityId) {
	ImGui::Text("Entity ID: %u", entityId);

	ImGuiArchive archive;
	std::vector<std::string> componentTypeNames = entityManager_->GetComponentTypeNames(entityId);
	std::sort(componentTypeNames.begin(), componentTypeNames.end());
	for (const std::string& componentTypeName : componentTypeNames) {
		ImGui::PushID(componentTypeName.c_str());
		if (ImGui::CollapsingHeader(componentTypeName.c_str())) {
			entityManager_->ReflectionComponentByName(entityId, componentTypeName, archive);
		}
		ImGui::PopID();
	}
}

void QFE::EDITOR::Inspector::AddComponentUI(uint32_t entityId, EditorCommandList& commandList) {
	// コンポーネントの追加と削除のUIを描画する
	if(ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::Button("Remove Component")) {
		ImGui::OpenPopup("RemoveComponentPopup");
	}

	// コンポーネントの追加と削除のポップアップメニューを描画する
	if (ImGui::BeginPopup("AddComponentPopup")) {
		std::vector<std::string> componentTypeNames = entityManager_->GetAllComponentTypeNames();
		// アルファベット順にソートする
		std::sort(componentTypeNames.begin(), componentTypeNames.end());
		for (const auto& componentTypeName : componentTypeNames) {
			if (ImGui::MenuItem(componentTypeName.c_str())) {
				commandList.AddCommand(std::make_unique<AddComponentCommand>(entityId, entityManager_, componentTypeName));
			}
		}
		ImGui::EndPopup();
	}
	
	// コンポーネントの削除のポップアップメニューを描画する
	if (ImGui::BeginPopup("RemoveComponentPopup")) {
		std::vector<std::string> componentTypeNames = entityManager_->GetComponentTypeNames(entityId);
		// アルファベット順にソートする
		std::sort(componentTypeNames.begin(), componentTypeNames.end());
		for (const auto& componentTypeName : componentTypeNames) {
			if (ImGui::MenuItem(componentTypeName.c_str())) {
				commandList.AddCommand(std::make_unique<DeleteComponentCommand>(entityId, entityManager_, componentTypeName));
			}
		}
		ImGui::EndPopup();
	}
}
