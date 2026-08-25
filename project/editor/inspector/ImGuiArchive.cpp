#include "ImGuiArchive.h"
#include "design-patterns/EntityManager.h"
#include "components/ObjectInfoComponent.h"
#include "components/TransformHierarchy.h"
#include "assetfactory/model/PrimitiveFactoryFuncs.h"

#include <filesystem>

namespace {
	const float kDragSpeed = 0.1f;
	// @brief フィールド名をラベルに変換する関数
	std::string MakeLabel(const std::string& name) {
		std::string label;
		label.reserve(name.size() + 4);

		for (size_t index = 0; index < name.size(); ++index) {
			const unsigned char current = static_cast<unsigned char>(name[index]);
			if (index > 0 && std::isupper(current) && name[index - 1] != '_') {
				label.push_back(' ');
			}
			label.push_back(name[index] == '_' ? ' ' : name[index]);
		}

		if (!label.empty()) {
			label[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(label[0])));
		}
		return label;
	}
	// @brief フィールド名に "color" が含まれているかを判定する関数
	bool IsColorField(const std::string& name) {
		std::string lowerName = name;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
			[](unsigned char character) { return static_cast<char>(std::tolower(character)); });
		return lowerName.find("color") != std::string::npos;
	}

	std::vector<std::string> FindResourceFiles(const std::vector<std::string>& extensions, bool removeExtension) {
		std::vector<std::string> results;
		std::error_code error;
		const std::filesystem::path resourceRoot = "resources";
		if (!std::filesystem::exists(resourceRoot, error)) {
			return results;
		}

		for (std::filesystem::recursive_directory_iterator iterator(
			resourceRoot,
			std::filesystem::directory_options::skip_permission_denied,
			error), end;
			iterator != end;
			iterator.increment(error)) {
			if (error) {
				error.clear();
				continue;
			}
			if (!iterator->is_regular_file(error)) {
				continue;
			}

			std::string extension = iterator->path().extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char character) { return static_cast<char>(std::tolower(character)); });
			if (std::find(extensions.begin(), extensions.end(), extension) == extensions.end()) {
				continue;
			}

			std::filesystem::path relativePath =
				std::filesystem::relative(iterator->path(), resourceRoot, error);
			if (error) {
				error.clear();
				continue;
			}
			if (removeExtension) {
				relativePath.replace_extension();
			}
			results.push_back(relativePath.generic_string());
		}

		std::sort(results.begin(), results.end());
		results.erase(std::unique(results.begin(), results.end()), results.end());
		return results;
	}

	void DrawResourceCombo(
		const std::string& label,
		std::string& value,
		const std::vector<std::string>& builtInOptions,
		const std::vector<std::string>& extensions,
		bool removeExtension,
		const char* emptyLabel = nullptr) {
		const char* preview = value.empty() && emptyLabel != nullptr ? emptyLabel : value.c_str();
		if (!ImGui::BeginCombo(label.c_str(), preview)) {
			return;
		}
		if (emptyLabel != nullptr && ImGui::Selectable(emptyLabel, value.empty())) {
			value.clear();
		}
		std::vector<std::string> options = builtInOptions;
		std::vector<std::string> resourceFiles = FindResourceFiles(extensions, removeExtension);
		options.insert(options.end(), resourceFiles.begin(), resourceFiles.end());
		for (const std::string& option : options) {
			const bool selected = value == option;
			if (ImGui::Selectable(option.c_str(), selected)) {
				value = option;
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	void DrawJsonValue(const char* label, nlohmann::json& value) {
		if (value.is_boolean()) {
			bool edited = value.get<bool>();
			if (ImGui::Checkbox(label, &edited)) value = edited;
			return;
		}
		if (value.is_number()) {
			float edited = value.get<float>();
			if (ImGui::DragFloat(label, &edited, kDragSpeed)) value = edited;
			return;
		}
		if (value.is_string()) {
			std::string edited = value.get<std::string>();
			if (ImGui::InputText(label, &edited)) value = edited;
			return;
		}

		std::string edited = value.dump();
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::InputText(label, &edited, ImGuiInputTextFlags_EnterReturnsTrue)) {
			nlohmann::json parsed = nlohmann::json::parse(edited, nullptr, false);
			if (!parsed.is_discarded()) value = std::move(parsed);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("JSON value. Press Enter to apply.");
		}
	}

	nlohmann::json MakeDefaultEventTrack() {
		return {
			{ "targetUuid", "" },
			{ "component", "TransformComponent" },
			{ "property", "transform.translate.x" },
			{ "interpolation", "Linear" },
			{ "keyframes", nlohmann::json::array({
				{ { "time", 0.0f }, { "value", 0.0f } },
				{ { "time", 1.0f }, { "value", 1.0f } }
			}) }
		};
	}

	void DrawEventTracks(nlohmann::json& tracks) {
		if (!tracks.is_array()) tracks = nlohmann::json::array();
		ImGui::TextUnformatted("Tracks");
		int trackToRemove = -1;
		for (size_t trackIndex = 0; trackIndex < tracks.size(); ++trackIndex) {
			auto& track = tracks[trackIndex];
			if (!track.is_object()) track = MakeDefaultEventTrack();
			ImGui::PushID(static_cast<int>(trackIndex));
			const std::string header = "Track " + std::to_string(trackIndex);
			if (ImGui::TreeNode(header.c_str())) {
				std::string targetUuid = track.value("targetUuid", std::string{});
				std::string component = track.value("component", std::string{});
				std::string property = track.value("property", std::string{});
				if (ImGui::InputText("Target UUID", &targetUuid)) track["targetUuid"] = targetUuid;
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Leave empty to target this entity.");
				if (ImGui::InputText("Component", &component)) track["component"] = component;
				if (ImGui::InputText("Property Path", &property)) track["property"] = property;

				std::string interpolation = track.value("interpolation", std::string("Linear"));
				if (ImGui::BeginCombo("Interpolation", interpolation.c_str())) {
					for (const char* option : { "Linear", "Step" }) {
						if (ImGui::Selectable(option, interpolation == option)) track["interpolation"] = option;
					}
					ImGui::EndCombo();
				}

				auto& keyframes = track["keyframes"];
				if (!keyframes.is_array()) keyframes = nlohmann::json::array();
				int keyframeToRemove = -1;
				for (size_t keyIndex = 0; keyIndex < keyframes.size(); ++keyIndex) {
					auto& keyframe = keyframes[keyIndex];
					if (!keyframe.is_object()) keyframe = { { "time", 0.0f }, { "value", 0.0f } };
					ImGui::PushID(static_cast<int>(keyIndex));
					float time = keyframe.value("time", 0.0f);
					ImGui::SetNextItemWidth(100.0f);
					if (ImGui::DragFloat("Time", &time, 0.01f, 0.0f)) keyframe["time"] = time;
					ImGui::SameLine();
					if (!keyframe.contains("value")) keyframe["value"] = 0.0f;
					ImGui::SetNextItemWidth(180.0f);
					DrawJsonValue("Value", keyframe["value"]);
					ImGui::SameLine();
					if (ImGui::SmallButton("Remove")) keyframeToRemove = static_cast<int>(keyIndex);
					ImGui::PopID();
				}
				if (keyframeToRemove >= 0) keyframes.erase(keyframes.begin() + keyframeToRemove);
				if (ImGui::Button("Add Keyframe")) {
					const float time = keyframes.empty() ? 0.0f : keyframes.back().value("time", 0.0f) + 1.0f;
					const nlohmann::json value = keyframes.empty() ? nlohmann::json(0.0f) : keyframes.back()["value"];
					keyframes.push_back({ { "time", time }, { "value", value } });
				}
				ImGui::SameLine();
				if (ImGui::Button("Remove Track")) trackToRemove = static_cast<int>(trackIndex);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (trackToRemove >= 0) tracks.erase(tracks.begin() + trackToRemove);
		if (ImGui::Button("Add Track")) tracks.push_back(MakeDefaultEventTrack());
	}

	nlohmann::json MakeDefaultCollisionAction() {
		return {
			{ "timing", "Enter" },
			{ "target", "Self" },
			{ "targetUuid", "" },
			{ "component", "EventComponent" },
			{ "property", "requestFlags" },
			{ "operation", "SetBits" },
			{ "value", 1u }
		};
	}

	void DrawStringCombo(const char* label, nlohmann::json& object, const char* key,
		const std::initializer_list<const char*>& options, const char* defaultValue) {
		std::string value = object.value(key, std::string(defaultValue));
		if (!ImGui::BeginCombo(label, value.c_str())) return;
		for (const char* option : options) {
			const bool selected = value == option;
			if (ImGui::Selectable(option, selected)) object[key] = option;
			if (selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	void DrawComponentTypeCombo(QFE::EntityManager* entityManager, nlohmann::json& action) {
		std::string component = action.value("component", std::string{});
		const char* preview = component.empty() ? "Select Component" : component.c_str();
		if (!ImGui::BeginCombo("Component", preview)) return;

		if (entityManager != nullptr) {
			std::vector<std::string> componentTypes = entityManager->GetAllComponentTypeNames();
			std::sort(componentTypes.begin(), componentTypes.end());
			for (const std::string& componentType : componentTypes) {
				const bool selected = component == componentType;
				if (ImGui::Selectable(componentType.c_str(), selected)) {
					action["component"] = componentType;
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	void DrawCollisionActions(QFE::EntityManager* entityManager, nlohmann::json& actions) {
		if (!actions.is_array()) actions = nlohmann::json::array();
		ImGui::TextUnformatted("Collision Actions");
		int actionToRemove = -1;
		for (size_t actionIndex = 0; actionIndex < actions.size(); ++actionIndex) {
			auto& action = actions[actionIndex];
			if (!action.is_object()) action = MakeDefaultCollisionAction();
			ImGui::PushID(static_cast<int>(actionIndex));
			const std::string header = "Action " + std::to_string(actionIndex);
			if (ImGui::TreeNode(header.c_str())) {
				DrawStringCombo("Timing", action, "timing", { "Enter", "Stay", "Exit" }, "Enter");
				DrawStringCombo("Target", action, "target",
					{ "Self", "Other", "EntityReference" }, "Self");
				if (action.value("target", std::string("Self")) == "EntityReference") {
					std::string targetUuid = action.value("targetUuid", std::string{});
					if (ImGui::InputText("Target UUID", &targetUuid)) action["targetUuid"] = targetUuid;
				}

				std::string property = action.value("property", std::string{});
				DrawComponentTypeCombo(entityManager, action);
				if (ImGui::InputText("Property Path", &property)) action["property"] = property;
				DrawStringCombo("Operation", action, "operation",
					{ "Set", "Add", "Multiply", "Toggle", "SetBits", "ClearBits" }, "Set");
				if (!action.contains("value")) action["value"] = 0.0f;
				DrawJsonValue("Value", action["value"]);

				if (ImGui::Button("Remove Action")) actionToRemove = static_cast<int>(actionIndex);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (actionToRemove >= 0) actions.erase(actions.begin() + actionToRemove);
		if (ImGui::Button("Add Action")) actions.push_back(MakeDefaultCollisionAction());
	}
}

QFE::EDITOR::ImGuiArchive::ImGuiArchive(EntityManager* entityManager, uint32_t entityId) :
	entityManager_(entityManager), entityId_(entityId) {
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, bool& value) {
	ImGui::Checkbox(MakeLabel(name).c_str(), &value);
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, float& value) {
	ImGui::DragFloat(MakeLabel(name).c_str(), &value, kDragSpeed);
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, int32_t& value) {
	ImGui::DragScalar(MakeLabel(name).c_str(), ImGuiDataType_S32, &value, 1.0f);
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, uint32_t& value) {
	ImGui::DragScalar(MakeLabel(name).c_str(), ImGuiDataType_U32, &value, 1.0f);
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, std::string& value) {
	if (name == "modelName") {
		DrawResourceCombo(
			MakeLabel(name), value, QFE::ASSET::GetPrimitiveMeshNames(), { ".obj" }, true);
		return;
	}
	if (name == "textureName") {
		DrawResourceCombo(
			MakeLabel(name), value, {},
			{ ".png", ".jpg", ".jpeg", ".dds", ".tga", ".bmp" },
			false, "Auto (Embedded / White1x1)");
		return;
	}
	if (name == "clipName") {
		DrawResourceCombo(
			MakeLabel(name), value, {}, { ".anim" }, true, "None");
		return;
	}

	const ImGuiInputTextFlags flags =
		name == "uuid" ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;
	ImGui::InputText(MakeLabel(name).c_str(), &value, flags);
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, MATH::Vector2& value) {
	ImGui::DragFloat2(MakeLabel(name).c_str(), &value.x, kDragSpeed);
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, MATH::Vector3& value) {
	ImGui::DragFloat3(MakeLabel(name).c_str(), &value.x, kDragSpeed);
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, MATH::Vector4& value) {
	if (IsColorField(name)) {
		ImGui::ColorEdit4(MakeLabel(name).c_str(), &value.x);
	} else {
		ImGui::DragFloat4(MakeLabel(name).c_str(), &value.x, kDragSpeed);
	}
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, MATH::EulerTransform& value) {
	ImGui::PushID(name.c_str());
	ImGui::TextUnformatted(MakeLabel(name).c_str());
	ImGui::Indent();
	ImGui::DragFloat3("Position", &value.translate.x, kDragSpeed);
	ImGui::DragFloat3("Rotation", &value.rotate.x, kDragSpeed);
	ImGui::DragFloat3("Scale", &value.scale.x, kDragSpeed);
	ImGui::Unindent();
	ImGui::PopID();
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, MATH::Matrix4x4& value) {
	ImGui::PushID(name.c_str());
	ImGui::TextUnformatted(MakeLabel(name).c_str());
	ImGui::Indent();
	for (int row = 0; row < 4; ++row) {
		std::string rowLabel = "Row " + std::to_string(row);
		ImGui::DragFloat4(rowLabel.c_str(), &value.m[row][0], kDragSpeed);
	}
	ImGui::Unindent();
	ImGui::PopID();
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, MATH::Bit32& value) {
	ImGui::PushID(name.c_str());
	ImGui::TextUnformatted(MakeLabel(name).c_str());
	ImGui::Indent();
	ImGui::Text("Num %u", value.value);
	const float checkboxWidth = ImGui::GetFrameHeight();
	// 操作したいビット位置（0 〜 31）
	for(int bitPos = 0; bitPos < 32; ++bitPos) {
		bool isBitSet = (value.value & (1u << bitPos)) != 0;
		std::string checkboxLabel = "##" + std::to_string(bitPos);
		if (ImGui::Checkbox(checkboxLabel.c_str(), &isBitSet)) {
			if (isBitSet) {
				value.SetBit(bitPos);
			} else {
				value.ClearBit(bitPos);
			}
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Bit %d", bitPos);
		}

		if (bitPos < 31) {
			ImGui::SameLine();
			if (ImGui::GetContentRegionAvail().x < checkboxWidth) {
				ImGui::NewLine();
			}
		}
	}

	// すべてのビットを操作するボタン
	if (ImGui::Button("All Active")) {
		value.Fill();
	}
	if (ImGui::Button("All Inactive")) {
		value.Clear();
	}
	ImGui::NewLine();
	ImGui::Unindent();
	ImGui::PopID();
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, EntityReference& value) {
	std::string preview = "None";
	bool referenceResolved = value.IsEmpty();

	if (entityManager_ != nullptr && !value.IsEmpty()) {
		uint32_t referencedEntityId = 0;
		if (QFE::SCENE::TryGetEntityIdByUuid(*entityManager_, value.uuid, referencedEntityId) &&
			entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(referencedEntityId)) {
			const auto& info =
				entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(referencedEntityId);
			preview = info.name.empty() ? "Unnamed Entity" : info.name;
			referenceResolved = true;
		}
	}
	if (!referenceResolved) {
		preview = "Missing (" + value.uuid.substr(0, 8) + ")";
	}

	if (!ImGui::BeginCombo(MakeLabel(name).c_str(), preview.c_str())) {
		return;
	}

	const bool hasNoParent = value.IsEmpty();
	if (ImGui::Selectable("None", hasNoParent)) {
		value.Clear();
	}

	if (entityManager_ != nullptr) {
		for (const uint32_t candidateId : entityManager_->GetActiveEntityIds()) {
			if (candidateId == entityId_ ||
				!entityManager_->HasComponent<QFE::SCENE::ObjectInfoComponent>(candidateId) ||
				QFE::SCENE::WouldCreateParentCycle(*entityManager_, entityId_, candidateId)) {
				continue;
			}

			const auto& candidate =
				entityManager_->GetComponent<QFE::SCENE::ObjectInfoComponent>(candidateId);
			if (candidate.uuid.empty()) {
				continue;
			}

			const std::string visibleName =
				candidate.name.empty() ? "Unnamed Entity" : candidate.name;
			const std::string itemLabel =
				visibleName + " (" + candidate.uuid.substr(0, 8) + ")##" + candidate.uuid;
			const bool isSelected = value.uuid == candidate.uuid;
			if (ImGui::Selectable(itemLabel.c_str(), isSelected)) {
				value.uuid = candidate.uuid;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
	}
	ImGui::EndCombo();
}

void QFE::EDITOR::ImGuiArchive::Process(const std::string& name, nlohmann::json& value) {
	if (name == "tracks") {
		DrawEventTracks(value);
		return;
	}
	if (name == "actions") {
		DrawCollisionActions(entityManager_, value);
		return;
	}
	DrawJsonValue(MakeLabel(name).c_str(), value);
}
