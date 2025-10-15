#include "KeyConfigEdit.h"
#include "Input/InputInterface.h" 
#include "AppUtility/String/DirectInputToString.h"
#include <format>
#include <optional>

void KeyConfigEdit::Initialize() {
	isActive_ = false;
	inputBuf_[0] = '\0';
	name_ = "KeyConfig Edit";
}

void KeyConfigEdit::Update() {
}

void KeyConfigEdit::Draw() {
	if (!isActive_) {
		return;
	}
	InputInterface* input = InputInterface::GetInstance();
	KeyConfig& keyConfig = input->GetKeyConfigManager();
	const auto& keyMap = keyConfig.GetKeyMap();

    std::optional<std::pair<std::string, uint32_t>> keyToRemove;

    ImGui::Begin("KeyConfig Edit", &isActive_, ImGuiWindowFlags_NoDocking);
    for (const auto& [name, keys] : keyMap) {
        if (keys.empty()) continue;
        ImGui::Text("%s :", name.c_str());
        for (size_t i = 0; i < keys.size(); ++i) {
            ImGui::SameLine();
            std::string label = std::format("[ {} ]##{}_{}", DirectInputToString(keys[i]), name, i);
            if (ImGui::Button(label.c_str())) {
                ImGui::OpenPopup(label.c_str());
            }
            if (ImGui::BeginPopup(label.c_str())) {
                if (ImGui::MenuItem("Delete")) {
                    // 削除はループ後に
                    keyToRemove = std::make_pair(name, keys[i]);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Edit")) {
                    editActionName_ = name;
                    editKeyIndex_ = i;
                    isEditPopupOpen_ = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

	ImGui::Separator();

    // ここで削除を実行
    if (keyToRemove) {
        keyConfig.RemoveKey(keyToRemove->first, keyToRemove->second);
    }

    if (ImGui::Button("Add Key##KeyConfig")) {
		isAddPopupOpen_ = true;
    }
	ImGui::SameLine();
	if (ImGui::Button("Reset to Default##KeyConfig")) {
		keyConfig.Reset();
	}

	if (isAddPopupOpen_) {
		ImGui::OpenPopup("Add Key");
		ImGui::InputText("Action Name", inputBuf_, sizeof(inputBuf_));
		if (ImGui::Button("OK")) {
			if (inputBuf_[0] != '\0') {
				// キー入力待ちへ
				isAddPopupOpen_ = false;
				isEditPopupOpen_ = true;
				editActionName_ = inputBuf_;
				editKeyIndex_ = SIZE_MAX; // 新規追加を示す
				inputBuf_[0] = '\0';
			}
			ImGui::CloseCurrentPopup();
		}
	}

    // 編集ポップアップ
    if (isEditPopupOpen_) {
        ImGui::OpenPopup("Edit Key");
        isEditPopupOpen_ = false;
    }
    if (ImGui::BeginPopup("Edit Key")) {
        ImGui::Text("Press a key to assign...");
        if (input->IsAnyKeyPressed()) {
            uint32_t keyCode = input->GetKeyCodeTrigger();
            if (keyCode != 0) {
                // キーを上書き
                keyConfig.EditKey(editActionName_, editKeyIndex_, keyCode);
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

void KeyConfigEdit::Run() {
	isActive_ = !isActive_;
}

void KeyConfigEdit::AddKeyConfig(const char* actionName, uint32_t keyId) {
	InputInterface::GetInstance()->GetKeyConfigManager().AddKey(actionName, keyId);
}

void KeyConfigEdit::ClearKeyConfig(const char* actionName) {
	InputInterface::GetInstance()->GetKeyConfigManager().RemoveKey(actionName);
}
