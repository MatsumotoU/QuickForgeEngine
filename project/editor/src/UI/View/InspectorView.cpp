/**
 * @file InspectorView.cpp
 * @brief 選択されたエンティティの詳細情報（コンポーネント）を表示・編集するパネルの実装
 */

#include "editor/include/UI/View/InspectorView.h"
#include "editor/include/UI/View/HierarchyView.h"

#include "engine/include/core/Bridge/EditorEngineBridge.h"
#include <format>

using namespace QFE;

InspectorView::InspectorView() {
	isActive_ = true;
	SetName("Inspector");
	selectedEntityId_ = 0;

	if (EditorEngineBridge::GetEntityTemplateDirectoryPath) {
		std::string scriptPath = EditorEngineBridge::GetEntityTemplateDirectoryPath(); // Wait, script path?
	}
	// Note: Directory paths are now handled via bridge where possible.
	// For now, let's just make sure we don't call AssetManager directly if bridge exists.
	if (EditorEngineBridge::GetModelDirectoryPath) {
		modelList_.LoadFileList(EditorEngineBridge::GetModelDirectoryPath(), ".obj");
	}
}

void InspectorView::Initialize() {
}

void InspectorView::Update() {
	selectedEntityId_ = HierarchyView::selectedEntityId_;
}

/** @brief 描画 */
void InspectorView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	// オブジェクトの名前
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::SceneObjectData)) {
		std::string name = EditorEngineBridge::GetEntityName(selectedEntityId_);
		std::string tag = EditorEngineBridge::GetEntityTag(selectedEntityId_);

		ImGui::Text("Entity ID: %d", selectedEntityId_);
		// name
		char nameBuffer[256];
		strncpy_s(nameBuffer, sizeof(nameBuffer), name.c_str(), sizeof(nameBuffer) - 1);
		if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
			EditorEngineBridge::SetEntityName(selectedEntityId_, nameBuffer);
		}
		// tag
		char tagBuffer[256];
		strncpy_s(tagBuffer, sizeof(tagBuffer), tag.c_str(), sizeof(tagBuffer) - 1);
		if (ImGui::InputText("Tag", tagBuffer, sizeof(tagBuffer))) {
			EditorEngineBridge::SetEntityTag(selectedEntityId_, tagBuffer);
		}

		ImGui::Separator();
	} else {
		ImGui::Text("No entity selected");
		ImGui::End();
		return;
	}

	// Parent ID
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::ParentData)) {
		if (ImGui::CollapsingHeader("Parent")) {
			if (ImGui::Button("Delete")) {
				if (EditorEngineBridge::Unparent) {
					EditorEngineBridge::Unparent(selectedEntityId_);
				}
			}
		}
	}

	// Transform
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::Transform)) {
		TransformData transform = EditorEngineBridge::GetTransform(selectedEntityId_);
		if (ImGui::CollapsingHeader("Transform")) {
			bool changed = false;
			if (ImGui::DragFloat3("Transition", transform.translate, 0.1f)) changed = true;
			if (ImGui::DragFloat3("Rotation", transform.rotate, 0.1f)) changed = true;
			if (ImGui::DragFloat3("Scale", transform.scale, 0.1f)) changed = true;

			if (changed) {
				EditorEngineBridge::SetTransform(selectedEntityId_, transform);
			}
		}
	}

	// Model
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::ModelHandle)) {
		ModelRenderInfo info = EditorEngineBridge::GetModelRenderInfo(selectedEntityId_);
		if (ImGui::CollapsingHeader("Model")) {
			ImGui::Text("Model Name: %s", info.modelName.c_str());

			ImGui::Separator();

			for (int i = 0; i < info.meshes.size(); ++i) {
				ImGui::PushID(i);
				auto& mesh = info.meshes[i];
				if (ImGui::TreeNode(std::format("Mesh: {}", i).c_str())) {
					// Material
					if (ImGui::TreeNode("Material")) {
						bool materialChanged = false;
						if (ImGui::ColorEdit4("Color", mesh.materialColor)) materialChanged = true;
						if (ImGui::DragFloat("Shininess", &mesh.materialShininess, 1.0f, 1.0f, 128.0f)) materialChanged = true;
						if (materialChanged) {
							EditorEngineBridge::SetMeshMaterial(selectedEntityId_, i, mesh.materialColor, mesh.materialShininess);
						}
						ImGui::TreePop();
					}

					// Light
					if (ImGui::TreeNode("Light")) {
						bool lightChanged = false;
						if (ImGui::ColorEdit4("Light Color", mesh.lightColor)) lightChanged = true;
						if (ImGui::DragFloat3("Light Direction", mesh.lightDirection, 0.1f)) lightChanged = true;
						if (lightChanged) {
							EditorEngineBridge::SetMeshLight(selectedEntityId_, i, mesh.lightColor, mesh.lightDirection);
						}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			ImGui::Separator();
			ImGui::Text("Change Model:");
			modelList_.DrawCombo();
			std::string selectedModel;
			if (modelList_.GetSelectedFileName(selectedModel)) {
				if (EditorEngineBridge::ChangeModel) {
					EditorEngineBridge::ChangeModel(selectedEntityId_, selectedModel);
				}
			}
		}
	}
	// Particle
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::Particle)) {
		ParticleInfo info = EditorEngineBridge::GetParticleInfo(selectedEntityId_);
		if (ImGui::CollapsingHeader("Particle")) {
			ImGui::Text("Model Name: %s", info.modelName.c_str());
			ImGui::Text("Max Particle Count: %d", info.maxParticleCount);
		}
	}

	// Sprite
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::SpriteData)) {
		SpriteInfo info = EditorEngineBridge::GetSpriteInfo(selectedEntityId_);
		if (ImGui::CollapsingHeader("Sprite")) {
			ImGui::Text("Sprite Name: %s", info.fileName.c_str());
			if (ImGui::Checkbox("Is Billboard", &info.isBillboard)) {
				EditorEngineBridge::SetSpriteInfo(selectedEntityId_, info);
			}
		}
	}
	// Camera
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::CameraData)) {
		CameraInfo info = EditorEngineBridge::GetCameraInfo(selectedEntityId_);
		if (ImGui::CollapsingHeader("Camera")) {
			bool changed = false;
			if (ImGui::SliderFloat("FOV", &info.fov, 0.1f, 3.0f)) changed = true;
			if (ImGui::SliderFloat("NearZ", &info.nearZ, 0.01f, 10.0f)) changed = true;
			if (ImGui::SliderFloat("FarZ", &info.farZ, 10.0f, 1000.0f)) changed = true;

			if (changed) {
				EditorEngineBridge::SetCameraInfo(selectedEntityId_, info);
			}
		}

		if (EditorEngineBridge::GetDebugCameraEntityId) {
			if (EditorEngineBridge::GetDebugCameraEntityId() == selectedEntityId_) {
				ImGui::Text("This is Main Camera");
			}
		}
	}
	// Billboard (No data, just check)
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::Billboard)) {
		if (ImGui::CollapsingHeader("Billboard")) {
			if (ImGui::Button("Delete##Billboard")) {
				EditorEngineBridge::RemoveComponent(selectedEntityId_, ComponentType::Billboard);
			}
			ImGui::Text("Billboard Component is Enabled.");
		}
	}

	// CSスクリプト
	std::vector<std::string> csScripts = EditorEngineBridge::GetCsharpClassNames(selectedEntityId_);
	for (const auto& className : csScripts) {
		if (ImGui::CollapsingHeader(className.c_str())) {
			if (ImGui::Button("Delete##CSharpScript")) {
				EditorEngineBridge::RemoveCsharpScript(selectedEntityId_, className);
			}
		}
	}

	// スクリプト
	std::vector<ScriptInfo> luaScripts = EditorEngineBridge::GetLuaScripts(selectedEntityId_);
	if (!luaScripts.empty()) {
		if (ImGui::CollapsingHeader("Scripts##ScriptProperties")) {
			for (auto& sh : luaScripts) {
				if (ImGui::TreeNode(sh.name.c_str())) {
					ImGui::Text("Handle: %d", sh.handle);
					ImGui::Text("Priority: %d", sh.priority);
					ImGui::Separator();
					for (auto& p : sh.params) {
						std::string inputLabel = p.name + "##" + std::to_string(sh.handle);
						if (p.type == ScriptParamType::Int) {
							int v = std::stoi(p.value);
							if (ImGui::InputInt(inputLabel.c_str(), &v)) {
								EditorEngineBridge::SetLuaScriptParam(selectedEntityId_, sh.handle, p.name, std::to_string(v));
							}
						} else if (p.type == ScriptParamType::Float) {
							float v = std::stof(p.value);
							if (ImGui::InputFloat(inputLabel.c_str(), &v)) {
								EditorEngineBridge::SetLuaScriptParam(selectedEntityId_, sh.handle, p.name, std::to_string(v));
							}
						} else if (p.type == ScriptParamType::Bool) {
							bool v = (p.value == "true");
							if (ImGui::Checkbox(inputLabel.c_str(), &v)) {
								EditorEngineBridge::SetLuaScriptParam(selectedEntityId_, sh.handle, p.name, v ? "true" : "false");
							}
						} else if (p.type == ScriptParamType::String) {
							char buf[256];
							strncpy_s(buf, p.value.c_str(), sizeof(buf));
							if (ImGui::InputText(inputLabel.c_str(), buf, sizeof(buf))) {
								EditorEngineBridge::SetLuaScriptParam(selectedEntityId_, sh.handle, p.name, std::string(buf));
							}
						}
					}

					ImGui::TreePop();
				}

				// 右クリックでポップアップメニュー
				std::string popupLabel = "ScriptPopup" + std::to_string(sh.handle);
				if (ImGui::BeginPopupContextItem(popupLabel.c_str())) {
					if (ImGui::MenuItem("Remove")) {
						EditorEngineBridge::RemoveLuaScript(selectedEntityId_, sh.handle);
					}
					ImGui::EndPopup();
				}
			}
		}
	}
	// Force
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::PhysicsForce)) {
		ForceData force = EditorEngineBridge::GetForceData(selectedEntityId_);
		if (ImGui::CollapsingHeader("Force")) {
			if (ImGui::Button("Delete##Force")) {
				EditorEngineBridge::RemoveComponent(selectedEntityId_, ComponentType::PhysicsForce);
			}
			bool changed = false;
			if (ImGui::DragFloat3("Velocity", force.velocity, 0.1f)) changed = true;
			if (ImGui::DragFloat3("Acceleration", force.acceleration, 0.1f)) changed = true;
			if (ImGui::DragFloat("Mass", &force.mass, 0.1f, 0.1f)) changed = true;
			if (ImGui::DragFloat("Friction", &force.friction, 0.01f, 0.0f)) changed = true;
			if (ImGui::DragFloat("GravityStrength", &force.gravityStrength, 0.01f, 0.0f)) changed = true;
			if (ImGui::Checkbox("Use Gravity", &force.isGravity)) changed = true;

			if (changed) {
				EditorEngineBridge::SetForceData(selectedEntityId_, force);
			}
		}
	}
	// SphereColliderData
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::SphereCollider)) {
		SphereColliderInfo info = EditorEngineBridge::GetSphereColliderInfo(selectedEntityId_);
		if (ImGui::CollapsingHeader("SphereCollider")) {
			if (ImGui::Button("Delete##SphereCollider")) {
				EditorEngineBridge::RemoveComponent(selectedEntityId_, ComponentType::SphereCollider);
			}
			bool changed = false;
			if (ImGui::DragFloat3("Center", info.center, 0.1f)) changed = true;
			if (ImGui::DragFloat("Radius", &info.radius, 0.1f, 0.1f)) changed = true;
			if (ImGui::Checkbox("Is Trigger", &info.isTrigger)) changed = true;
			if (ImGui::Checkbox("Is Static", &info.isStatic)) changed = true;

			ImGui::Text("Collider Layer:");
			for (int i = 0; i < 8; i++) {
				bool bit = (info.colliderLayer & (1 << i)) != 0;
				if (ImGui::Checkbox(std::format("##{}", i).c_str(), &bit)) {
					if (bit) info.colliderLayer |= (1 << i);
					else info.colliderLayer &= ~(1 << i);
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();

			ImGui::Text("Event Collider Layer:");
			for (int i = 0; i < 8; i++) {
				bool bit = (info.eventColliderLayer & (1 << i)) != 0;
				if (ImGui::Checkbox(std::format("##event{}", i).c_str(), &bit)) {
					if (bit) info.eventColliderLayer |= (1 << i);
					else info.eventColliderLayer &= ~(1 << i);
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();

#ifdef QFE_OPTIMIZE_OFF
			if (ImGui::Checkbox("Debug Draw", &info.isDraw)) changed = true;
#endif // QFE_OPTIMIZE_OFF

			if (changed) {
				EditorEngineBridge::SetSphereColliderInfo(selectedEntityId_, info);
			}
		}
	}

	// AABBColliderData
	if (EditorEngineBridge::HasComponent(selectedEntityId_, ComponentType::AABBCollider)) {
		AABBColliderInfo info = EditorEngineBridge::GetAABBColliderInfo(selectedEntityId_);
		if (ImGui::CollapsingHeader("AABBCollider")) {
			if (ImGui::Button("Delete##AABBCollider")) {
				EditorEngineBridge::RemoveComponent(selectedEntityId_, ComponentType::AABBCollider);
			}
			bool changed = false;
			if (ImGui::DragFloat3("Center", info.center, 0.1f)) changed = true;
			if (ImGui::DragFloat3("Size", info.size, 0.1f)) changed = true;
			if (ImGui::Checkbox("Is Trigger", &info.isTrigger)) changed = true;
			if (ImGui::Checkbox("Is Static", &info.isStatic)) changed = true;

			ImGui::Text("Collider Layer:");
			for (int i = 0; i < 8; i++) {
				bool bit = (info.colliderLayer & (1 << i)) != 0;
				if (ImGui::Checkbox(std::format("##{}", i).c_str(), &bit)) {
					if (bit) info.colliderLayer |= (1 << i);
					else info.colliderLayer &= ~(1 << i);
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();

			ImGui::Text("Event Collider Layer:");
			for (int i = 0; i < 8; i++) {
				bool bit = (info.eventColliderLayer & (1 << i)) != 0;
				if (ImGui::Checkbox(std::format("##event{}", i).c_str(), &bit)) {
					if (bit) info.eventColliderLayer |= (1 << i);
					else info.eventColliderLayer &= ~(1 << i);
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();

#ifdef QFE_OPTIMIZE_OFF
			if (ImGui::Checkbox("Debug Draw", &info.isDraw)) changed = true;
#endif // QFE_OPTIMIZE_OFF

			if (changed) {
				EditorEngineBridge::SetAABBColliderInfo(selectedEntityId_, info);
			}
		}
	}

	// コンポーネントの追加
	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup")) {
		// Utility
		if (ImGui::BeginMenu("Utility")) {
			if (ImGui::MenuItem("Parent")) {
				EditorEngineBridge::AddComponent(selectedEntityId_, ComponentType::ParentData);
			}
			if (ImGui::MenuItem("BillBorad")) {
				EditorEngineBridge::AddComponent(selectedEntityId_, ComponentType::Billboard);
			}
			ImGui::EndMenu();
		}

		// SphereCollider
		if (ImGui::BeginMenu("Collider3D")) {
			if (ImGui::MenuItem("SphereCollider")) {
				EditorEngineBridge::AddComponent(selectedEntityId_, ComponentType::SphereCollider);
			}
			if (ImGui::MenuItem("AABBCollider")) {
				EditorEngineBridge::AddComponent(selectedEntityId_, ComponentType::AABBCollider);
			}

			ImGui::EndMenu();
		}
		// Force
		if (ImGui::BeginMenu("Physics")) {
			if (ImGui::MenuItem("Force")) {
				EditorEngineBridge::AddComponent(selectedEntityId_, ComponentType::PhysicsForce);
			}
			ImGui::EndMenu();
		}
		// CsharpScript
		if (ImGui::BeginMenu("CSharpScript")) {
			if (ImGui::BeginMenu("AddScript")) {
				std::vector<std::string> csClasses = EditorEngineBridge::GetAvailableCsharpClasses();
				for (const auto& className : csClasses) {
					if (ImGui::MenuItem(className.c_str())) {
						EditorEngineBridge::AddCsharpScript(selectedEntityId_, className);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		// LuaScript
		if (ImGui::BeginMenu("LuaScript")) {
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
				EditorEngineBridge::AddLuaScript(selectedEntityId_, selectedScript);
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
				EditorEngineBridge::CreateLuaScript(scriptBuffer_);
				EditorEngineBridge::AddLuaScript(selectedEntityId_, scriptBuffer_);
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
