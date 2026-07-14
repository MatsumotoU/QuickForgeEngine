#include "Inspector.h"
#include "design-patterns/EntityManager.h"
#include "components/AllComponent.h"
#include "EngineDefines.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"

#include <imgui_stdlib.h>

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
	ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY;
	// ObjectInfoComponentの表示
	if (ImGui::BeginChild("ObjectInfo", ImVec2(0, 0), child_flags)) {
		DrawObjectInfoComponent(selectedEntityId, commandList);
		ImGui::EndChild();
	}

	// AddComponentUIの表示
	if (ImGui::BeginChild("AddComponent", ImVec2(0, 0), child_flags)) {
		AddComponentUI(selectedEntityId, commandList);
		ImGui::EndChild();
	}

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

void QFE::EDITOR::Inspector::DrawObjectInfoComponent(uint32_t entityId, EditorCommandList& commandList) {
	// ObjectInfoComponentの表示
	if (entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(entityId)) {
		auto& objectInfoComp = entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId);
		ImGui::InputText("Name", &objectInfoComp.name);
	} else {
		ImGui::Text("Entity ID: %u", entityId);
		ImGui::Text("No ObjectInfoComponent found.");
	}

	// TransformComponentの表示
	if (entityManager_->HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
		auto& transformComp = entityManager_->GetComponent<QFE::SCENE::TransformComponent>(entityId);
		if (ImGui::CollapsingHeader("Transform")) {
			ImGui::DragFloat3("Position", &transformComp.transform.translate.x);
			ImGui::DragFloat3("Rotation", &transformComp.transform.rotate.x);
			ImGui::DragFloat3("Scale", &transformComp.transform.scale.x);
		}
	}

	// MaterialComponentの表示
	if (entityManager_->HasComponent<QFE::SCENE::MaterialComponent>(entityId)) {
		auto& materialComp = entityManager_->GetComponent<QFE::SCENE::MaterialComponent>(entityId);
		if (ImGui::CollapsingHeader("Material")) {
			ImGui::ColorEdit4("Albedo Color", &materialComp.albedoColor.x);
		}
	}

	// ModelRenderComponentの表示
	if (entityManager_->HasComponent<QFE::SCENE::ModelRenderComponent>(entityId)) {
		auto& modelRenderComp = entityManager_->GetComponent<QFE::SCENE::ModelRenderComponent>(entityId);
		if (ImGui::CollapsingHeader("Model Render")) {
			ImGui::InputText("Model Name", &modelRenderComp.modelName);
			ImGui::Text("Can Render: %s", modelRenderComp.canRender ? "Yes" : "No");
			if(modelRenderComp.canRender == false && !modelRenderComp.renderErrorMessage.empty()) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", modelRenderComp.renderErrorMessage.c_str());
			}
		}
	}

	// CameraComponentの表示
	if (entityManager_->HasComponent<QFE::SCENE::CameraComponent>(entityId)) {
		auto& cameraComp = entityManager_->GetComponent<QFE::SCENE::CameraComponent>(entityId);
		if (ImGui::CollapsingHeader("Camera")) {
			if (cameraComp.isMainCamera) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Main Camera");
			} else {
				if (ImGui::Button("SetMainCamera")) {
					entityManager_->Each<QFE::SCENE::CameraComponent>([&](uint32_t otherEntityId, QFE::SCENE::CameraComponent& otherCameraComp) {
						if (otherCameraComp.isMainCamera) {
							otherCameraComp.isMainCamera = false;
						}
						});
					cameraComp.isMainCamera = true;
				}
			}
			

			ImGui::DragFloat("FOV", &cameraComp.fovY_);
			ImGui::Separator();
			ImGui::DragFloat("Bottom", &cameraComp.bottom_);
			ImGui::DragFloat("Top", &cameraComp.top_);
			ImGui::DragFloat("Left", &cameraComp.left_);
			ImGui::DragFloat("Right", &cameraComp.right_);
			ImGui::Separator();
			ImGui::DragFloat("Near Plane", &cameraComp.nearZ_);
			ImGui::DragFloat("Far Plane", &cameraComp.farZ_);
		}
	}
}

void QFE::EDITOR::Inspector::AddComponentUI(uint32_t entityId, EditorCommandList& commandList) {
	if(ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::Button("Remove Component")) {
		ImGui::OpenPopup("RemoveComponentPopup");
	}

	if (ImGui::BeginPopup("AddComponentPopup")) {
		std::vector<std::string> componentTypeNames = entityManager_->GetAllComponentTypeNames();
		for (const auto& componentTypeName : componentTypeNames) {
			if (ImGui::MenuItem(componentTypeName.c_str())) {
				commandList.AddCommand(std::make_unique<AddComponentCommand>(entityId, entityManager_, componentTypeName));
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("RemoveComponentPopup")) {
		std::vector<std::string> componentTypeNames = entityManager_->GetComponentTypeNames(entityId);
		for (const auto& componentTypeName : componentTypeNames) {
			if (ImGui::MenuItem(componentTypeName.c_str())) {
				commandList.AddCommand(std::make_unique<DeleteComponentCommand>(entityId, entityManager_, componentTypeName));
			}
		}
		ImGui::EndPopup();
	}
	
}
