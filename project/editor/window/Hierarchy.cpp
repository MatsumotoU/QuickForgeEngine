#include "Hierarchy.h"
#include "design-patterns/EntityManager.h"
#include "components/AllComponent.h"
#include "EngineDefines.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"
#include "assetfactory/model/PrimitiveFactoryFuncs.h"

#include <imgui/imgui.h>

#include "framework/window/WindowsWindowFrameWork.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <vector>

namespace {
	std::vector<std::string> FindObjModels() {
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
			if (extension != ".obj") {
				continue;
			}

			std::filesystem::path relativePath =
				std::filesystem::relative(iterator->path(), resourceRoot, error);
			if (error) {
				error.clear();
				continue;
			}
			relativePath.replace_extension();
			models.push_back(relativePath.generic_string());
		}

		std::sort(models.begin(), models.end());
		models.erase(std::unique(models.begin(), models.end()), models.end());
		return models;
	}

	std::string MakeEntityName(const std::string& modelName) {
		return std::filesystem::path(modelName).filename().string();
	}
}

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

		entityManager_->GetComponentStorage<QFE::SCENE::ObjectInfoComponent>().Each([&](
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
		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("Empty Object")) {
				commandList.AddCommand(std::make_unique<CreateEntityCommand>(
					"New Object", QFE::MATH::Vector3(0, 0, 0), entityManager_));
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

			const std::vector<std::string> modelNames = FindObjModels();
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

const std::set<uint32_t>& QFE::EDITOR::Hierarchy::GetSelectedEntities() const {
	return hierarchySelectedEntities_;
}
