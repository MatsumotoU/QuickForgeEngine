#include "InspectorView.h"
#include "HierarchyView.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Assets/AssetManager.h"
#include "Scene/Data/SceneObjectData.h"
#include "Core/Math/Transform.h"

InspectorView::InspectorView() {
	isActive_ = true;
	name_ = "Inspector View";
	selectedEntityId_ = 0;
}

void InspectorView::Initialize() {
}

void InspectorView::Update() {
	selectedEntityId_ = HierarchyView::selectedEntityId_;
}

void InspectorView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(name_.c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	// オブジェクトの名前
	AssetManager* assetManager = AssetManager::GetInstance();
	if (assetManager->GetEntityManager()->HasComponent<SceneObjectData>(selectedEntityId_)) {
		SceneObjectData& sceneObjData = assetManager->GetEntityManager()->GetComponent<SceneObjectData>(selectedEntityId_);
		ImGui::Text("Entity");
		ImGui::Text("Entity ID: %d", selectedEntityId_);
		ImGui::Text("Name: %s", sceneObjData.name.c_str());
		ImGui::Separator();
	} else {
		ImGui::Text("No entity selected");
	}

	if (assetManager->GetEntityManager()->HasComponent<Transform>(selectedEntityId_)) {
		Transform& transform = assetManager->GetEntityManager()->GetComponent<Transform>(selectedEntityId_);
		ImGui::Text("Transform");
		ImGui::DragFloat3("Transition", &transform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
		ImGui::Separator();
	}

	ImGui::End();
}
