#include "InspectorView.h"
#include "HierarchyView.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Assets/AssetManager.h"
#include "Scene/SceneManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Scene/Data/SceneObjectData.h"
#include "Core/Math/Transform.h"
#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Assets/Script/Data/ScriptHandle.h"
#include "Physics/Force.h"
#include "Collider/Data/SphereColliderData.h"
#include "Assets/3DModel/Data/ModelHandle.h"
#include "Assets/Sprite/Data/SpriteData.h"
#include "Core/Math/ParentData.h"
#include "Camera/Data/CameraData.h"

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
		
		ImGui::Text("Entity ID: %d(%d)", selectedEntityId_,sceneObjData.uniqueId);
		// name
		char nameBuffer[256];
		strncpy_s(nameBuffer, sizeof(nameBuffer), sceneObjData.name.c_str(), sizeof(nameBuffer) - 1);
		if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
			sceneObjData.name = nameBuffer;
		}
		// tag
		char tagBuffer[256];
		strncpy_s(tagBuffer, sizeof(tagBuffer), sceneObjData.tag.c_str(), sizeof(tagBuffer) - 1);
		if (ImGui::InputText("Tag", tagBuffer, sizeof(tagBuffer))) {
			sceneObjData.tag = tagBuffer;
		}

		ImGui::Separator();
	} else {
		ImGui::Text("No entity selected");
		ImGui::End();
		return;
	}

	// Parent ID
	if (assetManager->GetEntityManager()->HasComponent<ParentData>(selectedEntityId_)) {
		ParentData& parentData = assetManager->GetEntityManager()->GetComponent<ParentData>(selectedEntityId_);
		ImGui::Text("Parent Unique ID: %d", parentData.parentId);
		if (ImGui::Button("Delete")) {
			SceneManager::GetInstance()->Unparent(selectedEntityId_);
		}
	}

	// Transform
	if (assetManager->GetEntityManager()->HasComponent<Transform>(selectedEntityId_)) {
		Transform& transform = assetManager->GetEntityManager()->GetComponent<Transform>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Transform")) {
			ImGui::DragFloat3("Transition", &transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
		}
	}

	// Model
	if (assetManager->GetEntityManager()->HasComponent<ModelHandle>(selectedEntityId_)) {
		ModelHandle& modelHandle = assetManager->GetEntityManager()->GetComponent<ModelHandle>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Model")) {
			ImGui::Text("Model Name: %s", modelHandle.modelName.c_str());
			ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
			for (auto& mesh : modelData->meshRenderDataHandles) {
				Material* material = assetManager->GetMaterialBufferManager()->GetBufferData(mesh.materialHandle);
				std::string label = "Color##" + std::to_string(mesh.materialHandle);
				ImGui::ColorEdit4(label.c_str(), &material->color.x);
			}
		}
	}
	// Sprite
	if (assetManager->GetEntityManager()->HasComponent<SpriteData>(selectedEntityId_)) {
		SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Sprite")) {
			ImGui::Text("Sprite Name: %s", spriteData.textureName.c_str());
			ImGui::Checkbox("Is Draw", &spriteData.isDraw);
			ImGui::DragInt("Layer##Drag", reinterpret_cast<int*>(&spriteData.layer), 1, 0);
			ImGui::DragFloat("Width", &spriteData.width, 1.0f, 1.0f);
			ImGui::DragFloat("Height", &spriteData.height, 1.0f, 1.0f);
			Material* material = assetManager->GetMaterialBufferManager()->GetBufferData(spriteData.materialBufferHandle);
			ImGui::ColorEdit4("Color", &material->color.x);
		}
	}
	// Camera
	if (assetManager->GetEntityManager()->HasComponent<CameraData>(selectedEntityId_)) {
		CameraData& cameraData = assetManager->GetEntityManager()->GetComponent<CameraData>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Camera")) {
			ImGui::SliderFloat("FovY", &cameraData.fovY_, 0.1f, 3.0f);
			ImGui::SliderFloat("NearZ", &cameraData.nearZ_, 0.01f, 10.0f);
			ImGui::SliderFloat("FarZ", &cameraData.farZ_, 10.0f, 1000.0f);
		}

		CameraManager* cameraManager = CameraManager::GetInstance();
		if (cameraManager->GetMainCameraIndex() == cameraData.handle_) {
			ImGui::Text("This is Main Camera");
		}
	}

	// スクリプト
	if (assetManager->GetEntityManager()->HasComponent<ScriptHandles>(selectedEntityId_)) {
		ScriptHandles& scriptHandle = assetManager->GetEntityManager()->GetComponent<ScriptHandles>(selectedEntityId_);

		if (ImGui::CollapsingHeader("Scripts##ScripPropaties")) {
			if (ImGui::Button("Delete##Scripts")) {
				assetManager->GetEntityManager()->RemoveComponent<ScriptHandles>(selectedEntityId_);
			}
			std::vector<uint32_t> eraseIndices;
			for (size_t i = 0; i < scriptHandle.scriptHandles_.size(); ++i) {
				LuaHandle& sh = scriptHandle.scriptHandles_[i];
				// リスト表示
				if (ImGui::TreeNode(sh.scriptName_.c_str())) {
					// スクリプトのパラメータ表示
					LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(sh.handle_);
					ImGui::Text("Handle: %d", sh.handle_);
					ImGui::Text("Entity ID: %d", script->GetBindEntityId());
					ImGui::Text("Can Run: %s", script->IsCanRun() ? "True" : "False");
					ImGui::Separator();
					for (std::string& val : script->GetGlobalValuesList()) {
						std::string inputLabel = val + "##" + std::to_string(i);
						sol::state* state = script->GetScript();
						sol::object obj = (*state)[val];
						if (obj.is<int>()) {
							int v = obj.as<int>();
							if (ImGui::InputInt(inputLabel.c_str(), &v)) {
								(*state)[val] = v;
								auto it = sh.intParams_.find(val);
								if (it != sh.intParams_.end()) {
									it->second = v;
								} else {
									sh.intParams_[val] = v;// 新規追加
								}
							}
						} else if (obj.is<float>()) {
							float v = obj.as<float>();
							if (ImGui::InputFloat(inputLabel.c_str(), &v)) {
								(*state)[val] = v;
								auto it = sh.floatParams_.find(val);
								if (it != sh.floatParams_.end()) {
									it->second = v;
								} else {
									sh.floatParams_[val] = v; // 新規追加
								}
							}
						} else if (obj.is<bool>()) {
							bool v = obj.as<bool>();
							if (ImGui::Checkbox(inputLabel.c_str(), &v)) {
								(*state)[val] = v;
								auto it = sh.boolParams_.find(val);
								if (it != sh.boolParams_.end()) {
									it->second = v;
								} else {
									sh.boolParams_[val] = v; // 新規追加
								}
							}
						} else if (obj.is<std::string>()) {
							std::string v = obj.as<std::string>();
							char buf[256];
							strcpy_s(buf, v.c_str());
							if (ImGui::InputText(inputLabel.c_str(), buf, sizeof(buf))) {
								(*state)[val] = std::string(buf);
								auto it = sh.stringParams_.find(val);
								if (it != sh.stringParams_.end()) {
									it->second = std::string(buf);
								} else {
									sh.stringParams_[val] = std::string(buf); // 新規追加
								}
							}
						}
					}

					ImGui::TreePop();
				}

				// 右クリックでポップアップメニュー
				std::string popupLabel = "ScriptPopup" + std::to_string(i);
				if (ImGui::BeginPopupContextItem(popupLabel.c_str())) {
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
		}

	}
	// Force
	if (assetManager->GetEntityManager()->HasComponent<Force>(selectedEntityId_)) {
		Force& force = assetManager->GetEntityManager()->GetComponent<Force>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Force")) {
			if (ImGui::Button("Delete##Force")) {
				assetManager->GetEntityManager()->RemoveComponent<Force>(selectedEntityId_);
			}
			ImGui::DragFloat3("Velocity", &force.velocity.x, 0.1f);
			ImGui::DragFloat3("Acceleration", &force.acceleration.x, 0.1f);
			ImGui::DragFloat("Mass", &force.mass, 0.1f, 0.1f);
			ImGui::DragFloat("Friction", &force.friction, 0.01f, 0.0f);
			ImGui::DragFloat("GravityStrength", &force.gravityStrength, 0.01f, 0.0f);
			ImGui::Checkbox("Use Gravity", &force.isGravity);
		}
	}
	// SphereColliderData
	if (assetManager->GetEntityManager()->HasComponent<SphereColliderData>(selectedEntityId_)) {
		SphereColliderData& sphereCollider = assetManager->GetEntityManager()->GetComponent<SphereColliderData>(selectedEntityId_);
		if (ImGui::CollapsingHeader("SphereCollider")) {
			if (ImGui::Button("Delete##SphereCollider")) {
				assetManager->GetEntityManager()->RemoveComponent<SphereColliderData>(selectedEntityId_);
			}
			ImGui::DragFloat3("Center", &sphereCollider.sphere.center.x, 0.1f);
			ImGui::DragFloat("Radius", &sphereCollider.sphere.radius, 0.1f, 0.1f);
			ImGui::Checkbox("Is Trigger", &sphereCollider.isTrigger);
			ImGui::Checkbox("Is Static", &sphereCollider.isStatic);
#ifdef _DEBUG
			ImGui::Checkbox("Debug Draw", &sphereCollider.isDraw);
#endif // _DEBUG
		}
	}

	// コンポーネントの追加
	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup")) {
		// SphereCollider
		if (ImGui::BeginMenu("Collider3D")) {
			if (ImGui::MenuItem("SphereCollider")) {
				if (!assetManager->GetEntityManager()->HasComponent<SphereColliderData>(selectedEntityId_)) {
					assetManager->GetEntityManager()->EmplaceComponent<SphereColliderData>(selectedEntityId_);
				}
			}
			ImGui::EndMenu();
		}
		// Force
		if (ImGui::BeginMenu("Physics")) {
			if (ImGui::MenuItem("Force")) {
				if (!assetManager->GetEntityManager()->HasComponent<Force>(selectedEntityId_)) {
					assetManager->GetEntityManager()->EmplaceComponent<Force>(selectedEntityId_);
				}
			}
			ImGui::EndMenu();
		}
		// Script
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
