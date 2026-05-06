/**
 * @file InspectorView.cpp
 * @brief 選択されたエンティティの詳細情報（コンポーネント）を表示・編集するパネルの実装
 */

#include "editor/include/UI/View/InspectorView.h"
#include "editor/include/UI/View/HierarchyView.h"

#include "engine/include/core/Bridge/EngineBridgeProvider.h"

#include <format>

namespace QFE {

	InspectorView::InspectorView() {
		isActive_ = true;
		SetName("Inspector");
		selectedEntityId_ = 0;

		QFE::IEngineBridge* bridge = QFE::BRIDGE::GetBridge();
		if (bridge) {
			std::string scriptPath = bridge->GetScriptDirectoryPath();
		}

		if (bridge) {
			modelList_.LoadFileList(bridge->GetModelDirectoryPath(), ".obj");
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

		// エンジンブリッジの取得
		QFE::IEngineBridge* bridge = QFE::BRIDGE::GetBridge();
		if(bridge == nullptr) {
			ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
			ImGui::Text("Engine bridge not available");
			ImGui::End();
			return;
		}

		// ウィンドウの開始
		ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		// オブジェクトの名前
		if (bridge->HasComponent(selectedEntityId_, ComponentType::SceneObjectData)) {
			std::string name = bridge->GetEntityName(selectedEntityId_);
			std::string tag = bridge->GetEntityTag(selectedEntityId_);

			ImGui::Text("Entity ID: %d", selectedEntityId_);
			// name
			char nameBuffer[256];
			strncpy_s(nameBuffer, sizeof(nameBuffer), name.c_str(), sizeof(nameBuffer) - 1);
			if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
				bridge->SetEntityName(selectedEntityId_, nameBuffer);
			}
			// tag
			char tagBuffer[256];
			strncpy_s(tagBuffer, sizeof(tagBuffer), tag.c_str(), sizeof(tagBuffer) - 1);
			if (ImGui::InputText("Tag", tagBuffer, sizeof(tagBuffer))) {
				bridge->SetEntityTag(selectedEntityId_, tagBuffer);
			}

			ImGui::Separator();
		}
		else {
			ImGui::Text("No entity selected");
			ImGui::End();
			return;
		}

		// Parent ID
		if (bridge->HasComponent(selectedEntityId_, ComponentType::ParentData)) {
			if (ImGui::CollapsingHeader("Parent")) {
				if (ImGui::Button("Delete")) {
					if (bridge) {
						bridge->Unparent(selectedEntityId_);
					}
				}
			}
		}

		// Transform
		if (bridge->HasComponent(selectedEntityId_, ComponentType::Transform)) {
			TransformData transform = bridge->GetTransform(selectedEntityId_);
			if (ImGui::CollapsingHeader("Transform")) {
				bool changed = false;
				if (ImGui::DragFloat3("Transition", transform.translate, 0.1f)) changed = true;
				if (ImGui::DragFloat3("Rotation", transform.rotate, 0.1f)) changed = true;
				if (ImGui::DragFloat3("Scale", transform.scale, 0.1f)) changed = true;

				if (changed) {
					bridge->SetTransform(selectedEntityId_, transform);
				}
			}
		}

		// Model
		if (bridge->HasComponent(selectedEntityId_, ComponentType::ModelHandle)) {
			ModelRenderInfo info = bridge->GetModelRenderInfo(selectedEntityId_);
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
								bridge->SetMeshMaterial(selectedEntityId_, i, mesh.materialColor, mesh.materialShininess);
							}
							ImGui::TreePop();
						}

						// Light
						if (ImGui::TreeNode("Light")) {
							bool lightChanged = false;
							if (ImGui::ColorEdit4("Light Color", mesh.lightColor)) lightChanged = true;
							if (ImGui::DragFloat3("Light Direction", mesh.lightDirection, 0.1f)) lightChanged = true;
							if (lightChanged) {
								bridge->SetMeshLight(selectedEntityId_, i, mesh.lightColor, mesh.lightDirection);
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
					if (bridge) {
						bridge->ChangeModel(selectedEntityId_, selectedModel);
					}
				}
			}
		}
		// Particle
		if (bridge->HasComponent(selectedEntityId_, ComponentType::Particle)) {
			ParticleInfo info = bridge->GetParticleInfo(selectedEntityId_);
			if (ImGui::CollapsingHeader("Particle")) {
				ImGui::Text("Model Name: %s", info.modelName.c_str());
				ImGui::Text("Max Particle Count: %d", info.maxParticleCount);
			}
		}

		// Sprite
		if (bridge->HasComponent(selectedEntityId_, ComponentType::SpriteData)) {
			SpriteInfo info = bridge->GetSpriteInfo(selectedEntityId_);
			if (ImGui::CollapsingHeader("Sprite")) {
				ImGui::Text("Sprite Name: %s", info.fileName.c_str());
				if (ImGui::Checkbox("Is Billboard", &info.isBillboard)) {
					bridge->SetSpriteInfo(selectedEntityId_, info);
				}
				if (ImGui::DragFloat("Width", &info.width, 0.1f)) {
					bridge->SetSpriteInfo(selectedEntityId_, info);
				}
				if (ImGui::DragFloat("Height", &info.height, 0.1f)) {
					bridge->SetSpriteInfo(selectedEntityId_, info);
				}
				if (ImGui::DragFloat2("Pivot", info.pivot, 0.01f, 0.0f, 1.0f)) {
					bridge->SetSpriteInfo(selectedEntityId_, info);
				}
			}
		}
		// Camera
		if (bridge->HasComponent(selectedEntityId_, ComponentType::CameraData)) {
			CameraInfo info = bridge->GetCameraInfo(selectedEntityId_);
			if (ImGui::CollapsingHeader("Camera")) {
				bool changed = false;
				if (ImGui::SliderFloat("FOV", &info.fov, 0.1f, 3.0f)) changed = true;
				if (ImGui::SliderFloat("NearZ", &info.nearZ, 0.01f, 10.0f)) changed = true;
				if (ImGui::SliderFloat("FarZ", &info.farZ, 10.0f, 1000.0f)) changed = true;

				if (changed) {
					bridge->SetCameraInfo(selectedEntityId_, info);
				}
			}

			if (bridge) {
				if (bridge->GetDebugCameraEntityId() == selectedEntityId_) {
					ImGui::Text("This is Main Camera");
				}
			}
		}
		// Billboard (No data, just check)
		if (bridge->HasComponent(selectedEntityId_, ComponentType::Billboard)) {
			if (ImGui::CollapsingHeader("Billboard")) {
				if (ImGui::Button("Delete##Billboard")) {
					bridge->RemoveComponent(selectedEntityId_, ComponentType::Billboard);
				}
				ImGui::Text("Billboard Component is Enabled.");
			}
		}

		// CSスクリプト
		std::vector<std::string> csScripts = bridge->GetCsharpClassNames(selectedEntityId_);
		for (const auto& className : csScripts) {
			if (ImGui::CollapsingHeader(className.c_str())) {
				if (ImGui::Button("Delete##CSharpScript")) {
					bridge->RemoveCsharpScript(selectedEntityId_, className);
				}
			}
		}

		// Force
		if (bridge->HasComponent(selectedEntityId_, ComponentType::PhysicsForce)) {
			ForceData force = bridge->GetForceData(selectedEntityId_);
			if (ImGui::CollapsingHeader("Force")) {
				if (ImGui::Button("Delete##Force")) {
					bridge->RemoveComponent(selectedEntityId_, ComponentType::PhysicsForce);
				}
				bool changed = false;
				if (ImGui::DragFloat3("Velocity", force.velocity, 0.1f)) changed = true;
				if (ImGui::DragFloat3("Acceleration", force.acceleration, 0.1f)) changed = true;
				if (ImGui::DragFloat("Mass", &force.mass, 0.1f, 0.1f)) changed = true;
				if (ImGui::DragFloat("Friction", &force.friction, 0.01f, 0.0f)) changed = true;
				if (ImGui::DragFloat("GravityStrength", &force.gravityStrength, 0.01f, 0.0f)) changed = true;
				if (ImGui::Checkbox("Use Gravity", &force.isGravity)) changed = true;

				if (changed) {
					bridge->SetForceData(selectedEntityId_, force);
				}
			}
		}
		// SphereColliderData
		if (bridge->HasComponent(selectedEntityId_, ComponentType::SphereCollider)) {
			SphereColliderInfo info = bridge->GetSphereColliderInfo(selectedEntityId_);
			if (ImGui::CollapsingHeader("SphereCollider")) {
				if (ImGui::Button("Delete##SphereCollider")) {
					bridge->RemoveComponent(selectedEntityId_, ComponentType::SphereCollider);
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
					bridge->SetSphereColliderInfo(selectedEntityId_, info);
				}
			}
		}

		// AABBColliderData
		if (bridge->HasComponent(selectedEntityId_, ComponentType::AABBCollider)) {
			AABBColliderInfo info = bridge->GetAABBColliderInfo(selectedEntityId_);
			if (ImGui::CollapsingHeader("AABBCollider")) {
				if (ImGui::Button("Delete##AABBCollider")) {
					bridge->RemoveComponent(selectedEntityId_, ComponentType::AABBCollider);
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
					bridge->SetAABBColliderInfo(selectedEntityId_, info);
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
					bridge->AddComponent(selectedEntityId_, ComponentType::ParentData);
				}
				if (ImGui::MenuItem("BillBorad")) {
					bridge->AddComponent(selectedEntityId_, ComponentType::Billboard);
				}
				ImGui::EndMenu();
			}

			// SphereCollider
			if (ImGui::BeginMenu("Collider3D")) {
				if (ImGui::MenuItem("SphereCollider")) {
					bridge->AddComponent(selectedEntityId_, ComponentType::SphereCollider);
				}
				if (ImGui::MenuItem("AABBCollider")) {
					bridge->AddComponent(selectedEntityId_, ComponentType::AABBCollider);
				}

				ImGui::EndMenu();
			}
			// Force
			if (ImGui::BeginMenu("Physics")) {
				if (ImGui::MenuItem("Force")) {
					bridge->AddComponent(selectedEntityId_, ComponentType::PhysicsForce);
				}
				ImGui::EndMenu();
			}
			// CsharpScript
			if (ImGui::BeginMenu("CSharpScript")) {
				if (ImGui::BeginMenu("AddScript")) {
					std::vector<std::string> csClasses = bridge->GetAvailableCsharpClasses();
					for (const auto& className : csClasses) {
						if (ImGui::MenuItem(className.c_str())) {
							bridge->AddCsharpScript(selectedEntityId_, className);
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

} // namespace QFE
