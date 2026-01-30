/**
 * @file InspectorView.cpp
 * @brief 選択されたエンティティの詳細情報（コンポーネント）を表示・編集するパネルの実装
 */

#include "editor/include/UI/View/InspectorView.h"
#include "editor/include/UI/View/HierarchyView.h"

#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptExecutor.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/physics/Force.h"
#include "engine/include/collider/Data/SphereColliderData.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/assets/Sprite/Data/SpriteData.h"
#include "engine/include/core/Math/ParentData.h"
#include "engine/include/camera/Data/CameraData.h"
#include "engine/include/collider/Data/AABBColliderData.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"
#include "engine/include/assets/Particle/Data/ParticleComponent.h"

using namespace QFE;

InspectorView::InspectorView() {
	isActive_ = true;
	name_ = "Inspector View";
	selectedEntityId_ = 0;
	// csharpScriptListの初期化を削除
	scriptList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts"), ".lua");
	modelList_.LoadFileList(AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Model"), ".obj");
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

	ImGui::Begin(name_.c_str(), &isActive_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	// オブジェクトの名前
	AssetManager* assetManager = AssetManager::GetInstance();
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<SceneObjectData>(selectedEntityId_)) {
		SceneObjectData& sceneObjData = SceneManager::GetInstance()->GetEntityManager()->GetComponent<SceneObjectData>(selectedEntityId_);

		ImGui::Text("Entity ID: %d(%d)", selectedEntityId_, sceneObjData.uniqueId);
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
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<ParentData>(selectedEntityId_)) {
		ParentData& parentData = SceneManager::GetInstance()->GetEntityManager()->GetComponent<ParentData>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Parent")) {
			ImGui::Text("Parent Unique ID: %d", parentData.parentId);
			if (ImGui::Button("Delete")) {
				SceneManager::GetInstance()->Unparent(selectedEntityId_);
			}
		}
	}

	// Transform
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(selectedEntityId_)) {
		Transform& transform = SceneManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Transform")) {
			ImGui::DragFloat3("Transition", &transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
		}
	}

	// Model
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<ModelHandle>(selectedEntityId_)) {
		ModelHandle& modelHandle = SceneManager::GetInstance()->GetEntityManager()->GetComponent<ModelHandle>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Model")) {
			ImGui::Text("Model Name: %s", modelHandle.modelName.c_str());
			ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
			for (auto& mesh : modelData->meshRenderDataHandles) {
				Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(mesh.materialHandle);
				DirectionalLight* lightData = assetManager->GetGpuBufferPool()->GetConstantBufferData<DirectionalLight>(mesh.lightBufferHandle);

				std::string label = "Color##" + std::to_string(mesh.materialHandle);
				ImGui::ColorEdit4(label.c_str(), &material->color.x);
				label = "PhongShading##" + std::to_string(mesh.materialHandle);
				ImGui::DragFloat(label.c_str(), &material->shininess, 1.0f, 1.0f, 128.0f);

				std::string lightLabel = "Light Color##" + std::to_string(mesh.lightBufferHandle);
				ImGui::ColorEdit4(lightLabel.c_str(), &lightData->color.x);
				std::string lightDirLabel = "Light Direction##" + std::to_string(mesh.lightBufferHandle);
				ImGui::DragFloat3(lightDirLabel.c_str(), &lightData->direction.x, 0.1f);
				lightData->direction = Vector3::Normalize(lightData->direction);
			}

			ImGui::Separator();
			ImGui::Text("Change Model:");
			modelList_.DrawCombo();
			std::string selectedModel;
			if (modelList_.GetSelectedFileName(selectedModel)) {
				SceneManager::GetInstance()->ChangeEntityModel(selectedEntityId_, selectedModel);
			}
		}
	}
	// Particle
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<ParticleComponent>(selectedEntityId_)) {
		ParticleComponent& particleComp = SceneManager::GetInstance()->GetEntityManager()->GetComponent<ParticleComponent>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Particle")) {
			ImGui::Text("Model Name: %s", particleComp.modelName.c_str());
			ImGui::Text("Max Particle Count: %d", particleComp.maxParticleCount);
		}
	}

	// Sprite
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<SpriteData>(selectedEntityId_)) {
		SpriteData& spriteData = SceneManager::GetInstance()->GetEntityManager()->GetComponent<SpriteData>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Sprite")) {
			ImGui::Text("Sprite Name: %s", spriteData.textureName.c_str());
			ImGui::Checkbox("Is Draw", &spriteData.isDraw);
			ImGui::DragInt("Layer##Drag", reinterpret_cast<int*>(&spriteData.layer), 1, 0);
			ImGui::DragFloat("Width", &spriteData.width, 1.0f, 1.0f);
			ImGui::DragFloat("Height", &spriteData.height, 1.0f, 1.0f);
			ImGui::DragFloat2("Pivot", &spriteData.pivot.x, 0.01f);
			Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(spriteData.materialBufferHandle);
			ImGui::ColorEdit4("Color", &material->color.x);
		}
	}
	// Camera
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<CameraData>(selectedEntityId_)) {
		CameraData& cameraData = SceneManager::GetInstance()->GetEntityManager()->GetComponent<CameraData>(selectedEntityId_);
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
	// CSスクリプト
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<CsharpComponent>(selectedEntityId_)) {
		CsharpComponent& csharpComponent = SceneManager::GetInstance()->GetEntityManager()->GetComponent<CsharpComponent>(selectedEntityId_);
		for (const auto& csHandle : csharpComponent.csharpHandles_) {
			if (ImGui::CollapsingHeader(csHandle.className_.c_str())) {
				ImGui::Text("Script Index: %d", csHandle.scriptIndex_);
				if (ImGui::Button("Delete##CSharpScript")) {
					SceneManager::GetInstance()->GetEntityManager()->RemoveComponent<CsharpComponent>(selectedEntityId_);
				}
			}
		}
	}

	// スクリプト
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<ScriptHandles>(selectedEntityId_)) {
		ScriptHandles& scriptHandle = SceneManager::GetInstance()->GetEntityManager()->GetComponent<ScriptHandles>(selectedEntityId_);

		if (ImGui::CollapsingHeader("Scripts##ScriptProperties")) {
			if (ImGui::Button("Delete##Scripts")) {
				SceneManager::GetInstance()->GetEntityManager()->RemoveComponent<ScriptHandles>(selectedEntityId_);
			}
			std::vector<uint32_t> eraseIndices;
			for (size_t i = 0; i < scriptHandle.scriptHandles_.size(); ++i) {
				LuaHandle& sh = scriptHandle.scriptHandles_[i];
				// リスト表示
				if (ImGui::TreeNode(sh.scriptName_.c_str())) {
					// スクリプトのパラメータ表示
					LuaScriptOnQFE* script = SceneManager::GetInstance()->GetLuaScriptExecutor()->GetScript(sh.handle_);
					ImGui::Text("Handle: %d", sh.handle_);
					ImGui::Text("Entity ID: %d", script->GetBindEntityId());
					ImGui::Text("Can Run: %s", script->IsCanRun() ? "True" : "False");
					ImGui::DragInt("Priority", reinterpret_cast<int*>(&sh.priority_), 1, 0);
					ImGui::Separator();
					for (std::string& val : script->GetGlobalValuesList()) {
						std::string inputLabel = val + "##" + std::to_string(i);
						sol::environment& env = script->GetEnvironment();
						sol::object obj = env[val];
						if (obj.is<int>()) {
							int v = obj.as<int>();
							if (ImGui::InputInt(inputLabel.c_str(), &v)) {
								env[val] = v;
								auto it = sh.intParams_.find(val);
								if (it != sh.intParams_.end()) {
									it->second = v;
								} else {
									sh.intParams_[val] = v;// 譁ｰ隕剰ｿｽ蜉
								}
							}
						} else if (obj.is<float>()) {
							float v = obj.as<float>();
							if (ImGui::InputFloat(inputLabel.c_str(), &v)) {
								env[val] = v;
								auto it = sh.floatParams_.find(val);
								if (it != sh.floatParams_.end()) {
									it->second = v;
								} else {
									sh.floatParams_[val] = v; // 譁ｰ隕剰ｿｽ蜉
								}
							}
						} else if (obj.is<bool>()) {
							bool v = obj.as<bool>();
							if (ImGui::Checkbox(inputLabel.c_str(), &v)) {
								env[val] = v;
								auto it = sh.boolParams_.find(val);
								if (it != sh.boolParams_.end()) {
									it->second = v;
								} else {
									sh.boolParams_[val] = v; // 譁ｰ隕剰ｿｽ蜉
								}
							}
						} else if (obj.is<std::string>()) {
							std::string v = obj.as<std::string>();
							char buf[256];
							strcpy_s(buf, v.c_str());
							if (ImGui::InputText(inputLabel.c_str(), buf, sizeof(buf))) {
								env[val] = std::string(buf);
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
						// TODO: UtirityにVSCodeで開く処理を追加予定
					}
					if (ImGui::MenuItem("Remove")) {
						// スクリプト削除処理
						eraseIndices.push_back(static_cast<uint32_t>(i));
						SceneManager::GetInstance()->GetLuaScriptExecutor()->RemoveScript(sh.handle_);
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
				SceneManager::GetInstance()->GetEntityManager()->RemoveComponent<ScriptHandles>(selectedEntityId_);
			}
		}

	}
	// Force
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<Force>(selectedEntityId_)) {
		Force& force = SceneManager::GetInstance()->GetEntityManager()->GetComponent<Force>(selectedEntityId_);
		if (ImGui::CollapsingHeader("Force")) {
			if (ImGui::Button("Delete##Force")) {
				SceneManager::GetInstance()->GetEntityManager()->RemoveComponent<Force>(selectedEntityId_);
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
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<SphereColliderData>(selectedEntityId_)) {
		SphereColliderData& sphereCollider = SceneManager::GetInstance()->GetEntityManager()->GetComponent<SphereColliderData>(selectedEntityId_);
		if (ImGui::CollapsingHeader("SphereCollider")) {
			if (ImGui::Button("Delete##SphereCollider")) {
				SceneManager::GetInstance()->GetEntityManager()->RemoveComponent<SphereColliderData>(selectedEntityId_);
			}
			ImGui::DragFloat3("Center", &sphereCollider.sphere.center.x, 0.1f);
			ImGui::DragFloat("Radius", &sphereCollider.sphere.radius, 0.1f, 0.1f);
			ImGui::Checkbox("Is Trigger", &sphereCollider.isTrigger);
			ImGui::Checkbox("Is Static", &sphereCollider.isStatic);

			ImGui::Text("Collider Layer:");
			bool colliderLayerBits[8];
			for (int i = 0; i < 8; i++) {
				colliderLayerBits[i] = (sphereCollider.colliderLayer & (1 << i)) != 0;
			}
			bool changed = false;
			for (int i = 0; i < 8; i++) {
				if (ImGui::Checkbox(std::format("##{}", i).c_str(), &colliderLayerBits[i])) {
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
			if (changed) {
				sphereCollider.colliderLayer = 0;
				for (int i = 0; i < 8; i++) {
					if (colliderLayerBits[i]) {
						sphereCollider.colliderLayer |= (1 << i);
					}
				}
			}

			ImGui::Text("Event Collider Layer:");
			bool eventColliderLayerBits[8];
			for (int i = 0; i < 8; i++) {
				eventColliderLayerBits[i] = (sphereCollider.eventColliderLayer & (1 << i)) != 0;
			}
			changed = false;
			for (int i = 0; i < 8; i++) {
				if (ImGui::Checkbox(std::format("##event{}", i).c_str(), &eventColliderLayerBits[i])) {
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
			if (changed) {
				sphereCollider.eventColliderLayer = 0;
				for (int i = 0; i < 8; i++) {
					if (eventColliderLayerBits[i]) {
						sphereCollider.eventColliderLayer |= (1 << i);
					}
				}
			}
#ifdef QFE_OPTIMIZE_OFF
			ImGui::Checkbox("Debug Draw", &sphereCollider.isDraw);
#endif // _DEBUG
		}
	}

	// AABBColliderData
	if (SceneManager::GetInstance()->GetEntityManager()->HasComponent<AABBColliderData>(selectedEntityId_)) {
		AABBColliderData& aabbCollider = SceneManager::GetInstance()->GetEntityManager()->GetComponent<AABBColliderData>(selectedEntityId_);
		if (ImGui::CollapsingHeader("AABBCollider")) {
			if (ImGui::Button("Delete##AABBCollider")) {
				SceneManager::GetInstance()->GetEntityManager()->RemoveComponent<AABBColliderData>(selectedEntityId_);
			}
			ImGui::DragFloat3("Center", &aabbCollider.aabb.center.x, 0.1f);
			ImGui::DragFloat3("Size", &aabbCollider.aabb.size.x, 0.1f);
			ImGui::Checkbox("Is Trigger", &aabbCollider.isTrigger);
			ImGui::Checkbox("Is Static", &aabbCollider.isStatic);
			ImGui::Text("Collider Layer:");
			bool colliderLayerBits[8];
			for (int i = 0; i < 8; i++) {
				colliderLayerBits[i] = (aabbCollider.colliderLayer & (1 << i)) != 0;
			}
			bool changed = false;
			for (int i = 0; i < 8; i++) {
				if (ImGui::Checkbox(std::format("##{}", i).c_str(), &colliderLayerBits[i])) {
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
			if (changed) {
				aabbCollider.colliderLayer = 0;
				for (int i = 0; i < 8; i++) {
					if (colliderLayerBits[i]) {
						aabbCollider.colliderLayer |= (1 << i);
					}
				}
			}

			ImGui::Text("Event Collider Layer:");
			bool eventColliderLayerBits[8];
			for (int i = 0; i < 8; i++) {
				eventColliderLayerBits[i] = (aabbCollider.eventColliderLayer & (1 << i)) != 0;
			}
			changed = false;
			for (int i = 0; i < 8; i++) {
				if (ImGui::Checkbox(std::format("##event{}", i).c_str(), &eventColliderLayerBits[i])) {
					changed = true;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
			if (changed) {
				aabbCollider.eventColliderLayer = 0;
				for (int i = 0; i < 8; i++) {
					if (eventColliderLayerBits[i]) {
						aabbCollider.eventColliderLayer |= (1 << i);
					}
				}
			}
#ifdef QFE_OPTIMIZE_OFF
			ImGui::Checkbox("Debug Draw", &aabbCollider.isDraw);
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
				if (!SceneManager::GetInstance()->GetEntityManager()->HasComponent<SphereColliderData>(selectedEntityId_)) {
					SceneManager::GetInstance()->GetEntityManager()->EmplaceComponent<SphereColliderData>(selectedEntityId_);
				}
			}
			if (ImGui::MenuItem("AABBCollider")) {
				if (!SceneManager::GetInstance()->GetEntityManager()->HasComponent<AABBColliderData>(selectedEntityId_)) {
					SceneManager::GetInstance()->GetEntityManager()->EmplaceComponent<AABBColliderData>(selectedEntityId_);
				}
			}

			ImGui::EndMenu();
		}
		// Force
		if (ImGui::BeginMenu("Physics")) {
			if (ImGui::MenuItem("Force")) {
				if (!SceneManager::GetInstance()->GetEntityManager()->HasComponent<Force>(selectedEntityId_)) {
					SceneManager::GetInstance()->GetEntityManager()->EmplaceComponent<Force>(selectedEntityId_);
				}
			}
			ImGui::EndMenu();
		}
		// CsharpScript
		if (ImGui::BeginMenu("CSharpScript")) {
			if (ImGui::MenuItem("NewScript")) {
				// TODO: 新規C#スクリプト作成機能
			}

			if (ImGui::BeginMenu("AddScript")) {
				// C#繧ｯ繝ｩ繧ｹ繝ｪ繧ｹ繝医ｒ蜿門ｾ・
				csharpScriptClasses_ = SceneManager::GetInstance()->GetCsharpScriptExecutor()->GetAvailableScriptClasses();
				for (const auto& className : csharpScriptClasses_) {
					if (ImGui::MenuItem(className.c_str())) {
						SceneManager::GetInstance()->AddCsharpScript(selectedEntityId_, className);
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
				SceneManager::GetInstance()->AddLuaScript(selectedEntityId_, selectedScript);
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
				SceneManager::GetInstance()->AddLuaScript(selectedEntityId_, scriptBuffer_);
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
