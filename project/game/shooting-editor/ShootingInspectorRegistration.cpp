#include "ShootingInspectorRegistration.h"

#include "editor/inspector/ComponentInspectorRegistry.h"
#include "core/design-patterns/EntityManager.h"
#include "game/shooting/components/BulletEmitterComponent.h"
#include "game/shooting/components/InputBulletEmitterTriggerComponent.h"
#include "game/shooting/components/PeriodicBulletEmitterTriggerComponent.h"

#include <imgui/imgui.h>
#include <imgui_stdlib.h>

#include <string>
#include <utility>

namespace {
	constexpr float kDragSpeed = 0.1f;

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

	void DrawBulletEmitterInspector(QFE::EntityManager& entityManager, uint32_t entityId) {
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

	void DrawInputTriggerInspector(QFE::EntityManager& entityManager, uint32_t entityId) {
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
				ImGui::InputText("Input Action", &trigger.inputActionName);
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

	void DrawPeriodicTriggerInspector(QFE::EntityManager& entityManager, uint32_t entityId) {
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
	return
		registry.Register("BulletEmitterComponent", &DrawBulletEmitterInspector) &&
		registry.Register("InputBulletEmitterTriggerComponent", &DrawInputTriggerInspector) &&
		registry.Register("PeriodicBulletEmitterTriggerComponent", &DrawPeriodicTriggerInspector);
}
