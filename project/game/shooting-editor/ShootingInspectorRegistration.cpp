#include "ShootingInspectorRegistration.h"

#include "editor/inspector/ComponentInspectorRegistry.h"
#include "ecs/entity/EntityManager.h"
#include "game/shooting/components/BulletEmitterComponent.h"
#include "game/shooting/components/InputBulletEmitterTriggerComponent.h"
#include "game/shooting/components/PeriodicBulletEmitterTriggerComponent.h"
#include "game/shooting/components/EncounterRoomComponent.h"
#include "components/TransformHierarchy.h"
#include "components/AABBColliderComponent.h"
#include "components/OBBColliderComponent.h"
#include "components/SphereColliderComponent.h"
#include "framework/application/ColliderDebugDraw.h"
#include "framework/gui/GuiCoreFramework.h"

#include <imgui/imgui.h>
#include <imgui_stdlib.h>

#include <string>
#include <utility>

namespace {
	constexpr float kDragSpeed = 0.1f;

	void DrawHealthAction(nlohmann::json& action) {
		std::string source = action.value("amountSource", std::string("Fixed"));
		const char* preview = source == "OtherAttackPower" ? "Other's attack power" : "Fixed";
		if (ImGui::BeginCombo("Amount Source", preview)) {
			if (ImGui::Selectable("Fixed", source == "Fixed")) action["amountSource"] = "Fixed";
			if (ImGui::Selectable("Other's attack power", source == "OtherAttackPower")) action["amountSource"] = "OtherAttackPower";
			ImGui::EndCombo();
		}
		if (action.value("amountSource", std::string("Fixed")) == "Fixed") {
			uint32_t amount = action.value("amount", uint32_t{ 1 });
			if (!action.contains("amount")) action["amount"] = amount;
			if (ImGui::InputScalar("Amount", ImGuiDataType_U32, &amount)) action["amount"] = amount;
		} else {
			ImGui::TextDisabled("Uses the status of the other object in the collision.");
		}
	}

	void DrawEncounterRooms(QFE::EntityManager& entities, const QFE::MATH::Matrix4x4& viewProjection,
		const ImVec2& imagePosition, const ImVec2& imageSize) {
		using namespace QFE;
		auto* draw = ImGui::GetWindowDrawList();
		std::vector<FRAMEWORK::DebugLine> lines;
		draw->PushClipRect(imagePosition, { imagePosition.x + imageSize.x, imagePosition.y + imageSize.y }, true);
		const auto project = [&](const MATH::Vector3& position, ImVec2& screen) {
			const auto clip = MATH::Vector4::EulerTransform({ position.x, position.y, position.z, 1.0f }, viewProjection);
			if (clip.w <= 1.0e-5f || clip.z < 0.0f || clip.z > clip.w) return false;
			screen = { imagePosition.x + (clip.x / clip.w + 1.0f) * 0.5f * imageSize.x,
				imagePosition.y + (1.0f - clip.y / clip.w) * 0.5f * imageSize.y };
			return true;
		};
		const auto line = [&](const MATH::Vector3& start, const MATH::Vector3& end, ImU32 color) {
			const ImVec4 rgba = ImGui::ColorConvertU32ToFloat4(color);
			lines.push_back({ start, end, { rgba.x, rgba.y, rgba.z, rgba.w } });
		};
		const auto label = [&](const MATH::Vector3& position, const char* text, ImU32 color) {
			ImVec2 p;
			if (project(position, p)) draw->AddText(p, color, text);
		};
		entities.Each<STG::EncounterRoomComponent>([&](uint32_t id, STG::EncounterRoomComponent& room) {
			if (!entities.HasComponent<SCENE::TransformComponent>(id)) return;
			const auto world = SCENE::GetWorldMatrix(entities, id);
			MATH::EulerTransform bounds{};
			bounds.translate = { world.m[3][0], world.m[3][1], world.m[3][2] };
			const ImU32 roomColor = IM_COL32(255, 100, 230, 255);
			FRAMEWORK::ColliderDebugDraw::Box(bounds, room.halfExtents, false,
				[&](const auto& a, const auto& b) { line(a, b, roomColor); });
			label(bounds.translate, "Encounter entry area", roomColor);
			int missingDoors = 0;
			for (size_t index = 0; index < room.doors.size(); ++index) {
				const auto& setting = room.doors[index];
				uint32_t doorId = UINT32_MAX;
				if (!SCENE::TryGetEntityIdByUuid(entities, setting.entity.uuid, doorId)) { ++missingDoors; continue; }
				const auto* transform = entities.GetComponentPtr<SCENE::TransformComponent>(doorId);
				if (!transform) { ++missingDoors; continue; }
				auto closed = transform->transform;
				if (room.initialized && index < room.closedDoorPositions.size()) closed.translate = room.closedDoorPositions[index];
				auto opened = closed;
				opened.translate += setting.openOffset;
				const auto drawDoor = [&](const MATH::EulerTransform& pose, ImU32 color, const char* text) {
					const auto drawLine = [&](const auto& a, const auto& b) { line(a, b, color); };
					if (const auto* box = entities.GetComponentPtr<SCENE::AABBColliderComponent>(doorId))
						FRAMEWORK::ColliderDebugDraw::Box(pose, box->halfExtents, false, drawLine);
					else if (const auto* orientedBox = entities.GetComponentPtr<SCENE::OBBColliderComponent>(doorId))
						FRAMEWORK::ColliderDebugDraw::Box(pose, orientedBox->halfExtents, true, drawLine);
					else if (const auto* sphere = entities.GetComponentPtr<SCENE::SphereColliderComponent>(doorId))
						FRAMEWORK::ColliderDebugDraw::Sphere(pose.translate, sphere->radius, drawLine);
					else
						FRAMEWORK::ColliderDebugDraw::Box(pose, { 0.2f, 0.2f, 0.2f }, false, drawLine);
					label(pose.translate, text, color);
				};
				// 現行の衝突処理は扉のローカル Transform を使うため、同じ位置を表示する。
				drawDoor(closed, IM_COL32(255, 170, 40, 255), "Door closed");
				drawDoor(opened, IM_COL32(80, 220, 255, 255), "Door open");
				line(closed.translate, opened.translate, IM_COL32(255, 255, 255, 200));
			}
			if (missingDoors > 0) {
				ImVec2 p;
				if (project(bounds.translate, p)) {
					p.y += 16.0f;
					const std::string message = "Missing / unassigned doors: " + std::to_string(missingDoors);
					draw->AddText(p, IM_COL32(255, 90, 90, 255), message.c_str());
				}
			}
		});
		draw->PopClipRect();
		FRAMEWORK::DrawDebugLines(lines, viewProjection, { imagePosition.x, imagePosition.y },
			{ imageSize.x, imageSize.y }, false);
	}

	void DrawEmitterPatternCombo(
		const char* label,
		uint32_t& patternIndex,
		QFE::EntityManager& entityManager,
		uint32_t entityId) {
		const auto* emitter =
			entityManager.GetComponentPtr<QFE::STG::BulletEmitterComponent>(entityId);
		if (emitter == nullptr) {
			ImGui::DragScalar(label, ImGuiDataType_U32, &patternIndex, 1.0f);
			return;
		}

		std::string preview = "Missing Pattern " + std::to_string(patternIndex + 1);
		if (patternIndex < emitter->patterns.size()) {
			const std::string& patternName = emitter->patterns[patternIndex].name;
			preview = patternName.empty()
				? "Pattern " + std::to_string(patternIndex + 1)
				: patternName;
		}
		if (!ImGui::BeginCombo(label, preview.c_str())) {
			return;
		}
		for (std::size_t index = 0; index < emitter->patterns.size(); ++index) {
			const std::string& patternName = emitter->patterns[index].name;
			const std::string option =
				(patternName.empty() ? "Pattern " + std::to_string(index + 1) : patternName) +
				" (" + std::to_string(index + 1) + ")";
			const bool selected = patternIndex == index;
			if (ImGui::Selectable(option.c_str(), selected)) {
				patternIndex = static_cast<uint32_t>(index);
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		if (emitter->patterns.empty()) {
			ImGui::TextDisabled("No patterns registered");
		}
		ImGui::EndCombo();
	}

	void RemapEmitterPatternReferences(
		QFE::EntityManager& entityManager,
		uint32_t entityId,
		uint32_t removedIndex,
		uint32_t remainingPatternCount) {
		const auto remap = [removedIndex, remainingPatternCount](uint32_t& patternIndex) {
			if (remainingPatternCount == 0) {
				patternIndex = 0;
			} else if (patternIndex == removedIndex) {
				patternIndex = removedIndex < remainingPatternCount
					? removedIndex
					: remainingPatternCount - 1;
			} else if (patternIndex > removedIndex) {
				--patternIndex;
			}
		};

		if (auto* inputTriggers =
			entityManager.GetComponentPtr<QFE::STG::InputBulletEmitterTriggerComponent>(entityId)) {
			for (auto& trigger : inputTriggers->triggers) {
				remap(trigger.patternIndex);
			}
		}
		if (auto* periodicTriggers =
			entityManager.GetComponentPtr<QFE::STG::PeriodicBulletEmitterTriggerComponent>(entityId)) {
			for (auto& trigger : periodicTriggers->triggers) {
				remap(trigger.patternIndex);
			}
		}
	}

	void DrawBulletEmitterInspector(QFE::EntityManager& entityManager, uint32_t entityId,
		const std::vector<QFE::FRAMEWORK::GameInputAction>*) {
		auto& component = entityManager.GetComponent<QFE::STG::BulletEmitterComponent>(entityId);
		int patternToRemove = -1;
		for (std::size_t index = 0; index < component.patterns.size(); ++index) {
			ImGui::PushID(static_cast<int>(index));
			auto& pattern = component.patterns[index];
			const std::string header = pattern.name.empty()
				? "Pattern " + std::to_string(index + 1)
				: pattern.name;
			const bool expanded = ImGui::TreeNode(header.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove")) {
				patternToRemove = static_cast<int>(index);
			}
			if (expanded) {
				ImGui::InputText("Name", &pattern.name);
				ImGui::InputText("Bullet Prefab", &pattern.emitBulletName);
				ImGui::DragFloat3("Position Offset", &pattern.emitPos.x, kDragSpeed);
				ImGui::DragFloat3("Base Direction", &pattern.emitDir.x, kDragSpeed);
				ImGui::DragFloat("Radius", &pattern.emitRadius, kDragSpeed, 0.0f);
				ImGui::DragScalar("Bullet Count", ImGuiDataType_U32, &pattern.emitCount, 1.0f);
				ImGui::DragFloat("Polar Angle Step", &pattern.bulletAngleX, kDragSpeed);
				ImGui::DragFloat("Azimuth Angle Step", &pattern.bulletAngleY, kDragSpeed);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (patternToRemove >= 0) {
			component.patterns.erase(component.patterns.begin() + patternToRemove);
			RemapEmitterPatternReferences(
				entityManager,
				entityId,
				static_cast<uint32_t>(patternToRemove),
				static_cast<uint32_t>(component.patterns.size()));
		}
		if (ImGui::Button("Add Pattern")) {
			QFE::STG::BulletEmitterPattern pattern{};
			if (!component.patterns.empty()) {
				pattern = component.patterns.back();
			}
			pattern.name = "Pattern " + std::to_string(component.patterns.size() + 1);
			component.patterns.push_back(std::move(pattern));
		}
		ImGui::Checkbox("Emit Request", &component.emitRequest);
	}

	void DrawInputTriggerInspector(QFE::EntityManager& entityManager, uint32_t entityId,
		const std::vector<QFE::FRAMEWORK::GameInputAction>* actions) {
		auto& component =
			entityManager.GetComponent<QFE::STG::InputBulletEmitterTriggerComponent>(entityId);
		int triggerToRemove = -1;
		for (std::size_t index = 0; index < component.triggers.size(); ++index) {
			ImGui::PushID(static_cast<int>(index));
			auto& trigger = component.triggers[index];
			const std::string header = "Input Trigger " + std::to_string(index + 1);
			const bool expanded = ImGui::TreeNode(header.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove")) {
				triggerToRemove = static_cast<int>(index);
			}
			if (expanded) {
				ImGui::Checkbox("Enabled", &trigger.enabled);
				if (actions && ImGui::BeginCombo("Input Action", trigger.inputActionName.empty() ? "None" : trigger.inputActionName.c_str())) {
					if (ImGui::Selectable("None", trigger.inputActionName.empty())) trigger.inputActionName.clear();
					for (const auto& action : *actions)
						if (ImGui::Selectable(action.name.c_str(), trigger.inputActionName == action.name)) trigger.inputActionName = action.name;
					ImGui::EndCombo();
				}
				ImGui::DragScalar("Mouse Button", ImGuiDataType_S32, &trigger.mouseButton, 1.0f);
				ImGui::DragScalar(
					"Gamepad Button Mask", ImGuiDataType_U32, &trigger.gamePadButton, 1.0f);

				const char* triggerModeNames[] = { "Press (repeat)", "Trigger", "Release" };
				const char* triggerModePreview = trigger.triggerMode < 3
					? triggerModeNames[trigger.triggerMode]
					: "Unknown";
				if (ImGui::BeginCombo("Trigger Mode", triggerModePreview)) {
					for (uint32_t mode = 0; mode < 3; ++mode) {
						const bool selected = trigger.triggerMode == mode;
						if (ImGui::Selectable(triggerModeNames[mode], selected)) {
							trigger.triggerMode = mode;
						}
						if (selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::DragFloat(
					"Repeat Interval", &trigger.repeatInterval, kDragSpeed, 0.0f);
				DrawEmitterPatternCombo(
					"Emitter Pattern", trigger.patternIndex, entityManager, entityId);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (triggerToRemove >= 0) {
			component.triggers.erase(component.triggers.begin() + triggerToRemove);
		}
		if (ImGui::Button("Add Input Trigger")) {
			component.triggers.emplace_back();
		}
	}

	void DrawPeriodicTriggerInspector(QFE::EntityManager& entityManager, uint32_t entityId,
		const std::vector<QFE::FRAMEWORK::GameInputAction>*) {
		auto& component =
			entityManager.GetComponent<QFE::STG::PeriodicBulletEmitterTriggerComponent>(entityId);
		int triggerToRemove = -1;
		for (std::size_t index = 0; index < component.triggers.size(); ++index) {
			ImGui::PushID(static_cast<int>(index));
			auto& trigger = component.triggers[index];
			const std::string header = "Periodic Trigger " + std::to_string(index + 1);
			const bool expanded = ImGui::TreeNode(header.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove")) {
				triggerToRemove = static_cast<int>(index);
			}
			if (expanded) {
				ImGui::Checkbox("Enabled", &trigger.enabled);
				ImGui::Checkbox("Emit On Start", &trigger.emitOnStart);
				ImGui::DragFloat("Interval", &trigger.interval, kDragSpeed, 0.0f);
				DrawEmitterPatternCombo(
					"Emitter Pattern", trigger.patternIndex, entityManager, entityId);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (triggerToRemove >= 0) {
			component.triggers.erase(component.triggers.begin() + triggerToRemove);
		}
		if (ImGui::Button("Add Periodic Trigger")) {
			component.triggers.emplace_back();
		}
	}
}

bool QFE::GAME::RegisterShootingGameInspectors(
	EDITOR::ComponentInspectorRegistry& registry) {
	registry.RegisterSceneOverlay(&DrawEncounterRooms);
	return
		registry.RegisterCollisionAction("IncreaseHealth", &DrawHealthAction) &&
		registry.RegisterCollisionAction("DecreaseHealth", &DrawHealthAction) &&
		registry.Register("BulletEmitterComponent", &DrawBulletEmitterInspector) &&
		registry.Register("InputBulletEmitterTriggerComponent", &DrawInputTriggerInspector) &&
		registry.Register("PeriodicBulletEmitterTriggerComponent", &DrawPeriodicTriggerInspector);
}
