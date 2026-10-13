#include "Hierarchy.h"
#include "design-patterns/EntityManager.h"
#include "components/AllComponent.h"
#include "EngineDefines.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"
#include "assetfactory/model/PrimitiveFactoryFuncs.h"
#include "scene/SceneManager.h"

#include <imgui/imgui.h>
#include <imgui_stdlib.h>

#include "framework/window/WindowsWindowFrameWork.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace {
	std::vector<std::string> FindModelAssets() {
		std::vector<std::string> models;
		std::error_code error;
		const std::filesystem::path resourceRoot = "resources";
		if (!std::filesystem::exists(resourceRoot, error)) {
			return models;
		}

		for (std::filesystem::recursive_directory_iterator iterator(
			resourceRoot,
			std::filesystem::directory_options::skip_permission_denied,
			error), end;
			iterator != end;
			iterator.increment(error)) {
			if (error) {
				error.clear();
				continue;
			}
			if (!iterator->is_regular_file(error)) {
				continue;
			}

			std::string extension = iterator->path().extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char character) { return static_cast<char>(std::tolower(character)); });
			if (extension != ".obj" && extension != ".glb") {
				continue;
			}

			std::filesystem::path relativePath =
				std::filesystem::relative(iterator->path(), resourceRoot, error);
			if (error) {
				error.clear();
				continue;
			}
			// Keep the extension so an OBJ with the same stem remains selectable.
			if (extension == ".obj") relativePath.replace_extension();
			models.push_back(relativePath.generic_string());
		}

		std::sort(models.begin(), models.end());
		models.erase(std::unique(models.begin(), models.end()), models.end());
		return models;
	}

	std::string MakeEntityName(const std::string& modelName) {
		if (modelName == "Primitive/PlaneHorizontal") {
			return "Horizontal Plane";
		}
		return std::filesystem::path(modelName).filename().string();
	}

	struct HierarchyEntity {
		uint32_t id;
		std::string name;
	};

	std::string MakeNextGroupName(const std::set<std::string>& existingGroups) {
		for (uint32_t index = 1; ; ++index) {
			const std::string candidate = "Group " + std::to_string(index);
			if (!existingGroups.contains(candidate)) {
				return candidate;
			}
		}
	}
}

QFE::EDITOR::Hierarchy::Hierarchy(QFE::SCENE::SceneManager* sceneManager)
	: sceneManager_(sceneManager),
	entityManager_(sceneManager != nullptr ? &sceneManager->GetCurrentSceneEntityManager() : nullptr),
	isActive_(true) {}

void QFE::EDITOR::Hierarchy::Initialize() {
	isActive_ = true;
}

void QFE::EDITOR::Hierarchy::Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_);
	isFocus_ = ImGui::IsWindowFocused();

	// エンティティマネージャーが null の場合は、エラーメッセージを表示して終了する
	if(entityManager_ == nullptr) {
		ImGui::Text("EntityManager is null.");
		ImGui::End();
		return;
	}

	std::vector<uint32_t> entityIds = entityManager_->GetActiveEntityIds();
	ImGui::Text("Active Entities: %zu", entityIds.size());

	std::map<std::string, std::vector<HierarchyEntity>> groupedEntities;
	std::vector<HierarchyEntity> ungroupedEntities;
	std::set<uint32_t> visibleEntities;
	std::set<std::string> existingGroups;
	for (uint32_t entityId : entityIds) {
		if (!entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(entityId)) {
			continue;
		}

		const QFE::SCENE::ObjectInfoComponent& objectInfo =
			entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId);
		HierarchyEntity entity{ entityId, objectInfo.name };
		visibleEntities.insert(entityId);
		if (objectInfo.hierarchyGroup.empty()) {
			ungroupedEntities.push_back(std::move(entity));
		} else {
			existingGroups.insert(objectInfo.hierarchyGroup);
			groupedEntities[objectInfo.hierarchyGroup].push_back(std::move(entity));
		}
	}

	// 削除されたエンティティの選択状態を残さない
	for (auto it = hierarchySelectedEntities_.begin(); it != hierarchySelectedEntities_.end();) {
		if (!visibleEntities.contains(*it)) {
			it = hierarchySelectedEntities_.erase(it);
		} else {
			++it;
		}
	}

	bool openGroupDialog = false;
	constexpr const char* kEntityPayloadType = "QFE_HIERARCHY_ENTITY";
	const auto acceptEntityDrop = [&](const std::string& targetGroup) {
		if (!ImGui::BeginDragDropTarget()) {
			return;
		}

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kEntityPayloadType)) {
			const size_t payloadCount = payload->DataSize / sizeof(uint32_t);
			const auto* draggedEntityIds = static_cast<const uint32_t*>(payload->Data);
			if (payload->IsDelivery()) {
				for (size_t index = 0; index < payloadCount; ++index) {
					const uint32_t draggedEntityId = draggedEntityIds[index];
					if (entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(draggedEntityId)) {
						entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(draggedEntityId).hierarchyGroup = targetGroup;
					}
				}
			}
		}
		ImGui::EndDragDropTarget();
	};

	const auto drawEntityRow = [&](const HierarchyEntity& entity) {
		const bool currentSelected = hierarchySelectedEntities_.contains(entity.id);
		const std::string label = entity.name + "##Entity" + std::to_string(entity.id);
		if (ImGui::Selectable(label.c_str(), currentSelected)) {
			if (ImGui::GetIO().KeyCtrl) {
				if (currentSelected) {
					hierarchySelectedEntities_.erase(entity.id);
				} else {
					hierarchySelectedEntities_.insert(entity.id);
				}
			} else {
				hierarchySelectedEntities_.clear();
				hierarchySelectedEntities_.insert(entity.id);
			}
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			cameraFocusRequest_ = entity.id;
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !currentSelected) {
			hierarchySelectedEntities_.clear();
			hierarchySelectedEntities_.insert(entity.id);
		}

		if (ImGui::BeginDragDropSource()) {
			std::vector<uint32_t> draggedEntityIds;
			if (hierarchySelectedEntities_.contains(entity.id)) {
				draggedEntityIds.assign(hierarchySelectedEntities_.begin(), hierarchySelectedEntities_.end());
			} else {
				draggedEntityIds.push_back(entity.id);
			}
			ImGui::SetDragDropPayload(
				kEntityPayloadType,
				draggedEntityIds.data(),
				draggedEntityIds.size() * sizeof(uint32_t));
			ImGui::Text("Moving %zu entit%s", draggedEntityIds.size(), draggedEntityIds.size() == 1 ? "y" : "ies");
			ImGui::EndDragDropSource();
		}
		acceptEntityDrop(entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(entity.id).hierarchyGroup);

		const std::string entityContextMenuId =
			"EntityContextMenu##" + std::to_string(entity.id);
		if (ImGui::BeginPopupContextItem(entityContextMenuId.c_str(), ImGuiPopupFlags_MouseButtonRight)) {
			if (hierarchySelectedEntities_.size() >= 2 && ImGui::MenuItem("Group Selected...")) {
				groupNameInput_ = MakeNextGroupName(existingGroups);
				openGroupDialog = true;
			}
			if (ImGui::MenuItem("Copy Entity")) {
				for (uint32_t entityId : hierarchySelectedEntities_) {
					commandList.AddCommand(std::make_unique<CopyEntityCommand>(entityId, entityManager_));
				}
			}
			if (ImGui::MenuItem("Delete Entity")) {
				for (uint32_t entityId : hierarchySelectedEntities_) {
					commandList.AddCommand(std::make_unique<DeleteEntityCommand>(entityId, entityManager_));
				}
			}
			const bool hasGroupedSelection = std::any_of(
				hierarchySelectedEntities_.begin(), hierarchySelectedEntities_.end(), [&](uint32_t entityId) {
					return entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(entityId) &&
						!entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId).hierarchyGroup.empty();
				});
			if (hasGroupedSelection && ImGui::MenuItem("Ungroup Selected")) {
				for (uint32_t entityId : hierarchySelectedEntities_) {
					if (entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(entityId)) {
						entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId).hierarchyGroup.clear();
					}
				}
			}
			ImGui::EndPopup();
		}

	};

	// EntityManagerから取得したエンティティをグループ見出しの下に表示
	ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("EntityList", ImVec2(0, 0), child_flags)) {
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		const std::string ungroupedLabel = "Ungrouped (" + std::to_string(ungroupedEntities.size()) + ")##UngroupedHeader";
		const bool ungroupedOpen = ImGui::CollapsingHeader(ungroupedLabel.c_str());
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Drop entities here to remove them from their groups.");
		}
		acceptEntityDrop("");
		if (ungroupedOpen) {
			ImGui::Indent();
			for (const HierarchyEntity& entity : ungroupedEntities) {
				drawEntityRow(entity);
			}
			ImGui::Unindent();
		}

		for (const auto& [groupName, entities] : groupedEntities) {
			ImGui::PushID(groupName.c_str());
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			const std::string groupLabel = groupName + " (" + std::to_string(entities.size()) + ")##GroupHeader";
			const bool groupOpen = ImGui::CollapsingHeader(groupLabel.c_str());
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Drop entities here to move them into this group. Right-click to delete the group.");
			}
			acceptEntityDrop(groupName);
			if (ImGui::BeginPopupContextItem("GroupContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
				if (ImGui::MenuItem("Delete Group")) {
					for (const HierarchyEntity& entity : entities) {
						entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(entity.id).hierarchyGroup.clear();
					}
				}
				ImGui::EndPopup();
			}
			if (groupOpen) {
				ImGui::Indent();
				for (const HierarchyEntity& entity : entities) {
					drawEntityRow(entity);
				}
				ImGui::Unindent();
			}
			ImGui::PopID();
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
			hierarchySelectedEntities_.clear();
		}

		if (ImGui::BeginPopupContextWindow(
			"HierarchyContextMenu",
			ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
			if (ImGui::BeginMenu("Create")) {
				if (ImGui::MenuItem("Camera")) {
					commandList.AddCommand(std::make_unique<CreateCameraCommand>(sceneManager_));
				}
				if (ImGui::MenuItem("Sky Box")) {
					commandList.AddCommand(std::make_unique<CreateEntityCommand>(
						"Sky Box", QFE::MATH::Vector3(0, 0, 0), entityManager_, std::string{}, false, true));
				}
				if (ImGui::MenuItem("Empty Object")) {
					commandList.AddCommand(std::make_unique<CreateEntityCommand>(
						"New Object", QFE::MATH::Vector3(0, 0, 0), entityManager_));
				}
				if (ImGui::MenuItem("Sprite")) {
					commandList.AddCommand(std::make_unique<CreateEntityCommand>(
						"New Sprite", QFE::MATH::Vector3(640.0f, 360.0f, 0.0f), entityManager_, std::string{}, true));
				}
				if (ImGui::BeginMenu("3D Object")) {
					for (const std::string& modelName : QFE::ASSET::GetPrimitiveMeshNames()) {
						const std::string entityName = MakeEntityName(modelName);
						if (ImGui::MenuItem(entityName.c_str())) {
							commandList.AddCommand(std::make_unique<CreateEntityCommand>(
								entityName, QFE::MATH::Vector3(0, 0, 0), entityManager_, modelName));
						}
					}
					ImGui::EndMenu();
				}
				const std::vector<std::string> modelNames = FindModelAssets();
				if (ImGui::BeginMenu("Model", !modelNames.empty())) {
					for (const std::string& modelName : modelNames) {
						if (ImGui::MenuItem(modelName.c_str())) {
							commandList.AddCommand(std::make_unique<CreateEntityCommand>(
								MakeEntityName(modelName), QFE::MATH::Vector3(0, 0, 0), entityManager_, modelName));
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		if (openGroupDialog) {
			ImGui::OpenPopup("CreateHierarchyGroupPopup");
		}
		if (ImGui::BeginPopupModal("CreateHierarchyGroupPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Group selected entities under:");
			ImGui::SetNextItemWidth(260.0f);
			const bool submitName = ImGui::InputText(
				"##HierarchyGroupName", &groupNameInput_, ImGuiInputTextFlags_EnterReturnsTrue);
			const size_t firstNonSpace = groupNameInput_.find_first_not_of(" \t\r\n");
			const size_t lastNonSpace = groupNameInput_.find_last_not_of(" \t\r\n");
			const bool hasValidName = firstNonSpace != std::string::npos;
			if ((ImGui::Button("Create", ImVec2(120, 0)) || submitName) && hasValidName) {
				groupNameInput_ = groupNameInput_.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
				for (uint32_t entityId : hierarchySelectedEntities_) {
					if (entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(entityId)) {
						entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId).hierarchyGroup = groupNameInput_;
					}
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

	}
	ImGui::EndChild();

	ImGui::Text("Selected Entities: %zu", hierarchySelectedEntities_.size());
	for(uint32_t entityId : hierarchySelectedEntities_) {
		ImGui::Text("Entity ID: %u", entityId);
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

const std::set<uint32_t>& QFE::EDITOR::Hierarchy::GetSelectedEntities() const {
	return hierarchySelectedEntities_;
}
