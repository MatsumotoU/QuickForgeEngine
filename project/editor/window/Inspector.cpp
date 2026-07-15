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

	// ScriptComponentの表示
	if (entityManager_->HasComponent<QFE::SCENE::ScriptComponent>(entityId)) {
		auto& scriptComp = entityManager_->GetComponent<QFE::SCENE::ScriptComponent>(entityId);
		if (ImGui::CollapsingHeader("Script")) {
			ImGui::InputText("Script Function Name", &scriptComp.scriptFunctionName);
		}
	}

	// HealthComponentの表示
	if(entityManager_->HasComponent<QFE::STG::HealthComponent>(entityId)) {
		auto& healthComp = entityManager_->GetComponent<QFE::STG::HealthComponent>(entityId);
		if (ImGui::CollapsingHeader("Health")) {
			int health = static_cast<int>(healthComp.maxHealth);
			ImGui::DragInt("Max Health", &health);
			healthComp.maxHealth = static_cast<uint32_t>(health);
		}
	}

	// ColliderComponentの表示
	if(entityManager_->HasComponent<QFE::SCENE::SphereColliderComponent>(entityId)) {
		auto& colliderComp = entityManager_->GetComponent<QFE::SCENE::SphereColliderComponent>(entityId);
		if (ImGui::CollapsingHeader("Collider")) {
			ImGui::DragFloat("Radius", &colliderComp.radius);
			ImGui::Text("Mask: 0x%X", colliderComp.mask);
			DrawBitmaskUI(colliderComp.mask);
		}
	}

	// PlayerComponentの表示
	if(entityManager_->HasComponent<QFE::STG::ShootingPlayerComponent>(entityId)) {
		auto& playerComp = entityManager_->GetComponent<QFE::STG::ShootingPlayerComponent>(entityId);
		if (ImGui::CollapsingHeader("Player")) {
			ImGui::DragFloat("Speed", &playerComp.speed);
			ImGui::DragFloat("Shoot Interval", &playerComp.shootInterval);
			ImGui::DragFloat("Bomb Interval", &playerComp.bombInterval);
			ImGui::DragFloat3("Bullet Spawn Offset", &playerComp.bulletSpawnOffset.x);
			ImGui::DragFloat3("Bomb Spawn Offset", &playerComp.bombSpawnOffset.x);
		}
	}

	// BulletComponentの表示
	if(entityManager_->HasComponent<QFE::STG::BulletComponent>(entityId)) {
		auto& bulletComp = entityManager_->GetComponent<QFE::STG::BulletComponent>(entityId);
		if (ImGui::CollapsingHeader("Bullet")) {
			ImGui::DragFloat("Speed", &bulletComp.speed);
			ImGui::DragFloat("Lifetime", &bulletComp.lifeTimeMax);
			ImGui::DragFloat3("Direction", &bulletComp.dir.x);

			// マスクの表示
			ImGui::Text("Mask: 0x%X", bulletComp.mask);
			DrawBitmaskUI(bulletComp.mask);
		}
	}

	// MoveLimitComponentの表示
	if(entityManager_->HasComponent<QFE::STG::MoveLimitComponent>(entityId)) {
		auto& moveLimitComp = entityManager_->GetComponent<QFE::STG::MoveLimitComponent>(entityId);
		if (ImGui::CollapsingHeader("Move Limit")) {
			ImGui::DragFloat3("Min Limit", &moveLimitComp.minLimit.x);
			ImGui::DragFloat3("Max Limit", &moveLimitComp.maxLimit.x);
			ImGui::DragFloat3("Center", &moveLimitComp.center.x);
			ImGui::DragFloat3("Auto Scroll Speed", &moveLimitComp.autoScrollSpeed.x);
			ImGui::DragFloat("Auto Scroll Distance", &moveLimitComp.autoScrollDistance);
		}
	}

	// TrackingComponentの表示
	if(entityManager_->HasComponent<QFE::STG::PlayerTrackingComponent>(entityId)) {
		auto& trackingComp = entityManager_->GetComponent<QFE::STG::PlayerTrackingComponent>(entityId);
		if (ImGui::CollapsingHeader("PlayerTrackingComponent")) {
			ImGui::DragFloat3("Tracking Offset Position", &trackingComp.trackingOffsetPos.x);
			ImGui::Checkbox("Tracking X", &trackingComp.isTrackingX);
			ImGui::Checkbox("Tracking Y", &trackingComp.isTrackingY);
			ImGui::Checkbox("Tracking Z", &trackingComp.isTrackingZ);
			ImGui::Checkbox("Tracking Rotation", &trackingComp.isTrackingRotation);
			ImGui::DragFloat3("Tracking Rotation Offset", &trackingComp.trackingRotationOffset.x,0.1f);
			ImGui::DragFloat("Tracking Rotation Transpose", &trackingComp.trackingRotationTranspose,0.1f);
		}
	}

	// AutoScrollComponentの表示
	if(entityManager_->HasComponent<QFE::STG::AutoScrollComponent>(entityId)) {
		auto& autoScrollComp = entityManager_->GetComponent<QFE::STG::AutoScrollComponent>(entityId);
		if (ImGui::CollapsingHeader("AutoScrollComponent")) {
			ImGui::DragFloat("Auto Scroll Speed", &autoScrollComp.speed);
			ImGui::DragFloat("Auto Scroll Distance", &autoScrollComp.distance);
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

void QFE::EDITOR::Inspector::DrawBitmaskUI(uint32_t& value) {
	ImGui::PushID(value); // 複数のエディタを置いたときのID衝突を避ける

	// 読みやすいように等幅フォントを使う（設定されている場合）
	ImGui::Text("Value: 0x%08X (%u)", value, value);
	// 右側（最下位ビット LSB）から左側（最上位ビット MSB）へ並べるか、
	// 左から右へ並べるかでループの順序を決めます。
	// ここでは、一般的な2進数表記に合わせて「左がMSB（Bit 31）、右がLSB（Bit 0）」で並べます。
	for (int i = 31; i >= 0; i--)
	{
		// 現在のビットの状態を取得
		bool bit = (value & (1U << i)) != 0;

		// チェックボックスのラベルを非表示にしつつ、内部的な識別子としてビット番号を使う
		char label[16];
		sprintf(label, "##bit_%d", i);

		// 各ビットのチェックボックスを描画
		if (ImGui::Checkbox(label, &bit))
		{
			if (bit)
			{
				value |= (1U << i);  // ビットを立てる
			} else
			{
				value &= ~(1U << i); // ビットを降ろす
			}
		}

		// ホバー時に何番目のビットかツールチップで表示する（地味に便利です）
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Bit %d (Value: %u)", i, 1U << i);
		}

		// 最後のビット以外は、横に並べる
		if (i > 0)
		{
			// 8ビット（1バイト）ごとに少し広めのスペースを空けて視認性を上げる
			if (i % 8 == 0)
			{
				ImGui::SameLine(0.0f, 12.0f); // 12pxのスペース
			} else
			{
				ImGui::SameLine(0.0f, 4.0f);  // 通常のスペース
			}
		}
	}

	ImGui::PopID();
}
