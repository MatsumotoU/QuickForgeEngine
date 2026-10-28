#include "Inspector.h"
#include "design-patterns/EntityManager.h"
#include "inspector/ComponentInspectorRegistry.h"
#include "inspector/ImGuiArchive.h"
#include "components/DirectionalLightComponent.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"

#include <unordered_set>

namespace {
	void DrawAddComponentItems(
		const std::vector<std::string>& componentTypeNames,
		const std::unordered_set<std::string>& existingComponentTypeNames,
		uint32_t entityId,
		QFE::EntityManager* entityManager,
		QFE::EDITOR::EditorCommandList& commandList) {
		for (const std::string& componentTypeName : componentTypeNames) {
			if (existingComponentTypeNames.contains(componentTypeName)) {
				continue;
			}

			if (ImGui::MenuItem(componentTypeName.c_str())) {
				commandList.AddCommand(std::make_unique<QFE::EDITOR::AddComponentCommand>(
					entityId, entityManager, componentTypeName));
			}
		}
	}
}

QFE::EDITOR::Inspector::Inspector(
	QFE::EntityManager* entityManager,
	const ComponentInspectorRegistry* componentInspectorRegistry) :
	entityManager_(entityManager),
	componentInspectorRegistry_(componentInspectorRegistry),
	isActive_(true),
	isFocus_(false) {
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

	ImGuiArchive archive(entityManager_, entityId);
	std::vector<std::string> componentTypeNames = entityManager_->GetComponentTypeNames(entityId);
	std::sort(componentTypeNames.begin(), componentTypeNames.end());
	for (const std::string& componentTypeName : componentTypeNames) {
		ImGui::PushID(componentTypeName.c_str());
		if (ImGui::CollapsingHeader(componentTypeName.c_str())) {
			const bool wasDrawnByCustomInspector =
				componentInspectorRegistry_ != nullptr &&
				componentInspectorRegistry_->Draw(componentTypeName, *entityManager_, entityId);
			if (!wasDrawnByCustomInspector) {
				if (componentTypeName == "DirectionalLightComponent") {
					auto& light = entityManager_->GetComponent<QFE::SCENE::DirectionalLightComponent>(entityId);
					archive.Process("direction", light.direction);
					if (ImGui::IsItemDeactivatedAfterEdit()) {
						light.direction = QFE::SCENE::DirectionalLightComponent::NormalizeDirection(light.direction);
					}
					archive.Process("intensity", light.intensity);
				} else {
					entityManager_->ReflectionComponentByName(entityId, componentTypeName, archive);
				}
			}
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
		const std::vector<std::string> existingComponents = entityManager_->GetComponentTypeNames(entityId);
		const std::unordered_set<std::string> existingComponentSet(
			existingComponents.begin(), existingComponents.end());

		if (ImGui::BeginTabBar("ComponentCategoryTabs", ImGuiTabBarFlags_FittingPolicyScroll)) {
			std::vector<std::string> allComponentTypeNames = entityManager_->GetAllComponentTypeNames();
			std::sort(allComponentTypeNames.begin(), allComponentTypeNames.end());
			if (ImGui::BeginTabItem("All")) {
				DrawAddComponentItems(
					allComponentTypeNames, existingComponentSet, entityId, entityManager_, commandList);
				ImGui::EndTabItem();
			}

			for (const std::string& category : entityManager_->GetAllComponentCategories()) {
				if (ImGui::BeginTabItem(category.c_str())) {
					std::vector<std::string> componentTypeNames =
						entityManager_->GetAllComponentTypeNames(category);
					std::sort(componentTypeNames.begin(), componentTypeNames.end());
					DrawAddComponentItems(
						componentTypeNames, existingComponentSet, entityId, entityManager_, commandList);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
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
