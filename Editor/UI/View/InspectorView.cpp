#include "InspectorView.h"
#include "HierarchyView.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Assets/AssetManager.h"
#include "Scene/SceneManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Scene/Data/SceneObjectData.h"
#include "Core/Math/Transform.h"
#include "Assets/AssetManager.h"
#include "Assets/Script/Data/ScriptHandle.h"
#include "Physics/Force.h"

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

		ImGui::Text("Scripts");
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			assetManager->GetEntityManager()->RemoveComponent<ScriptHandles>(selectedEntityId_);
		}
		std::vector<uint32_t> eraseIndices;
		for (size_t i = 0; i < scriptHandle.scriptHandles_.size(); ++i) {
			const auto& sh = scriptHandle.scriptHandles_[i];
			// リスト表示
			ImGui::Selectable(sh.scriptName_.c_str());

			// 右クリックでポップアップメニュー
			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Open in VSCode")) {
					LuaScriptResourceManager::GetInstance()->OpenAndEditScript(sh.scriptName_);
				}
				if (ImGui::MenuItem("Remove")) {
					// スクリプト削除処理 
					eraseIndices.push_back(static_cast<uint32_t>(i));
					LuaScriptResourceManager::GetInstance()->RequestRemoveScript(sh.handle_);
				}
				ImGui::EndPopup();
			}
		}
		// 後ろから削除してインデックスずれを防ぐ
		for (auto it = eraseIndices.rbegin(); it != eraseIndices.rend(); ++it) {
			scriptHandle.scriptHandles_.erase(scriptHandle.scriptHandles_.begin() + *it);
		}
		// スクリプトがなくなったらコンポーネントごと削除
		if (scriptHandle.scriptHandles_.size() <= 0) {
			assetManager->GetEntityManager()->RemoveComponent<ScriptHandles>(selectedEntityId_);
		}
		ImGui::Separator();
	}
	// Force
	if (assetManager->GetEntityManager()->HasComponent<Force>(selectedEntityId_)) {
		Force& force = assetManager->GetEntityManager()->GetComponent<Force>(selectedEntityId_);
		ImGui::Text("Force");
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			assetManager->GetEntityManager()->RemoveComponent<Force>(selectedEntityId_);
		}
		ImGui::DragFloat3("Velocity", &force.velocity.x, 0.1f);
		ImGui::DragFloat3("Acceleration", &force.acceleration.x, 0.1f);
		ImGui::DragFloat("Mass", &force.mass, 0.1f, 0.1f);
		ImGui::DragFloat("Friction", &force.friction, 0.01f, 0.0f);
		ImGui::DragFloat("GravityStrength", &force.gravityStrength, 0.01f, 0.0f);
		ImGui::Checkbox("Use Gravity", &force.isGravity);
		ImGui::Separator();
	}

	// コンポーネントの追加
	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup")) {
		if (ImGui::BeginMenu("Physics")) {
			if (ImGui::MenuItem("Force")) {
				if (!assetManager->GetEntityManager()->HasComponent<Force>(selectedEntityId_)) {
					assetManager->GetEntityManager()->EmplaceComponent<Force>(selectedEntityId_);
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Script")) {
			if (ImGui::MenuItem("NewScript")) {
				openScriptPopup_ = true;
				strcpy_s(scriptBuffer_, "NewScript.lua");
			}

			if (ImGui::BeginMenu("AddScript")) {
				scriptList_.DrawMenuItem();
				ImGui::EndMenu();
			}

			std::string selectedScript;
			if (scriptList_.GetSelectedFileName(selectedScript)) {
				SceneManager::GetInstance()->AddScript(selectedEntityId_, selectedScript);
			}


			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
	if (openScriptPopup_) {
		ImGui::OpenPopup("NewScript");
		if (ImGui::BeginPopupModal("NewScript", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputText("Script Name", scriptBuffer_, IM_ARRAYSIZE(scriptBuffer_));
			if (ImGui::Button("Create")) {
				LuaScriptResourceManager::GetInstance()->CreateScript(scriptBuffer_);
				SceneManager::GetInstance()->AddScript(selectedEntityId_, scriptBuffer_);
				openScriptPopup_ = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				openScriptPopup_ = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	ImGui::End();
}
