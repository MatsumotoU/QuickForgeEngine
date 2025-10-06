#include "DebugConsole.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Scene/SceneManager.h"

void DebugConsole::Initialize() {
	name_ = "DebugConsole";
	isActive_ = false;
}

void DebugConsole::Update() {
}

void DebugConsole::Draw() {
    if (!isActive_) { return; }

    ImGui::Begin("DebugConsole", &isActive_, ImGuiWindowFlags_NoDocking);

    // 履歴表示用の子ウィンドウ（上部）
    ImGui::BeginChild("History", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 8), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& item : items_) {
        ImGui::TextUnformatted(item.c_str());
    }
    ImGui::EndChild();

    // 入力欄（下部固定）
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("Input", inputBuf_, IM_ARRAYSIZE(inputBuf_),
        ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (inputBuf_[0]) {
            ExecCommand(inputBuf_);
            inputBuf_[0] = '\0';
        }
    }
    ImGui::SetItemDefaultFocus();

    ImGui::End();
}

void DebugConsole::Run() {
	isActive_ = true;
}

void DebugConsole::ExecCommand(const char* command) {
    // コマンド履歴に追加
    items_.emplace_back(std::string("> ") + command);

    // コマンドの簡単な例
    if (strcmp(command, "clear") == 0) {
        items_.clear();
    } else if (strcmp(command, "help") == 0) {
        items_.emplace_back("Available commands: help, clear, echo [text] , scp_init , scp_update , scene_run , scene_stop");
    } else if (strncmp(command, "echo ", 5) == 0) {
        items_.emplace_back(command + 5);
    } else if (strcmp(command, "scp_init") == 0) {
        items_.emplace_back("Run Init All Scripts.");
		LuaScriptResourceManager::GetInstance()->InitializeAllScripts();
    } else if (strcmp(command, "scp_update") == 0) {
        items_.emplace_back("Run Update All Scripts OneFrame.");
        LuaScriptResourceManager::GetInstance()->UpdateAllScripts();
    } else if (strcmp(command, "scene_run") == 0) {
        items_.emplace_back("Run Scene.");
		SceneManager::GetInstance()->StartScript();
    } else if (strcmp(command, "scene_stop") == 0) {
        items_.emplace_back("Stop Scene.");
        SceneManager::GetInstance()->StopScript();
    } else if (strcmp(command, "scp_reload") == 0) {
        items_.emplace_back("Reload All Scripts.");
		LuaScriptResourceManager::GetInstance()->ReloadAllScripts();
    } else {
        items_.emplace_back("Unknown command. Type 'help' for list.");
    }
}
