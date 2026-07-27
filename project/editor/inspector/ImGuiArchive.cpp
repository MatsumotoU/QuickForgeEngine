#include "ImGuiArchive.h"

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
	ImGui::InputText(MakeLabel(name).c_str(), &value);
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
