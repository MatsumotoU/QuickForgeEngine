#include "InspectorView.h"
#include "HierarchyView.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Assets/AssetManager.h"
#include "Scene/SceneManager.h"
#include "Scene/Data/SceneObjectData.h"
#include "Core/Math/Transform.h"
#include "Assets/AssetManager.h"
#include "Assets/Script/Data/ScriptHandle.h"

InspectorView::InspectorView() {
	isActive_ = true;
	name_ = "Inspector View";
	selectedEntityId_ = 0;
	scriptList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts"), ".lua");
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
		ImGui::End();
		return;
	}
	// Transform
	if (assetManager->GetEntityManager()->HasComponent<Transform>(selectedEntityId_)) {
		Transform& transform = assetManager->GetEntityManager()->GetComponent<Transform>(selectedEntityId_);
		ImGui::Text("Transform");
		ImGui::DragFloat3("Transition", &transform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
		ImGui::Separator();
	}
	// スクリプト
	if (assetManager->GetEntityManager()->HasComponent<ScriptHandles>(selectedEntityId_)) {
		ScriptHandles& scriptHandle = assetManager->GetEntityManager()->GetComponent<ScriptHandles>(selectedEntityId_);
		
		ImGui::Text("Script");
		for (const auto& sh : scriptHandle.scriptHandles_) {
			ImGui::Text("Name: %s", sh.scriptName_.c_str());
		}
		ImGui::Separator();
	}

	// コンポーネントの追加
	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup")) {
		if (ImGui::BeginMenu("NewScript")) {
			scriptList_.DrawMenuItem();
			std::string selectedScript;
			if (scriptList_.GetSelectedFileName(selectedScript)) {
				SceneManager::GetInstance()->AddScript(selectedEntityId_, selectedScript);
			}
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
