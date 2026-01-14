/**
 * @file DebugConsole.cpp
 * @brief エディタ内コマンドラインコンソールの実装
 */

#include "editor/include/UI/Edit/DebugConsole.h"
#include "assets/Script/LuaScriptResourceManager.h"
#include "scene/SceneManager.h"
#include "audio/AudioInterface.h"
#include "assets/Script/CsharpVirtualEnvironmentOnQFE.h"

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
	} else if (strncmp(command, "cs_create ", 10) == 0) {
		std::string projectName = command + 10;
		items_.emplace_back("Create C# Project: " + projectName);
		CsharpVirtualEnvironmentOnQFE::GetInstance()->CreateCSProject(projectName);
	} else if (strcmp(command, "cs_load") == 0) {
		items_.emplace_back("Load C# Assembly" );
		CsharpVirtualEnvironmentOnQFE::GetInstance()->LoadAssembly();
	} else if (strcmp(command, "cs_open") == 0) {
		items_.emplace_back("Open C# ScriptProject");
		CsharpVirtualEnvironmentOnQFE::GetInstance()->OpenCSharpProjectInVSCode();
	} else if (strcmp(command, "cs_compile") == 0) {
		items_.emplace_back("Compile C# Scripts");
		CsharpVirtualEnvironmentOnQFE::GetInstance()->CompileScripts();
	} else if (strncmp(command, "cs_ci ", 6) == 0) {
		std::string className = command + 6;
		items_.emplace_back("Create C# Script Instance: " + className);
		CsharpVirtualEnvironmentOnQFE::GetInstance()->CreateScriptInstance(className);
	} else if (strncmp(command, "cs_cie ", 7) == 0) {
		std::string entityIdStr = command + 7;
		std::string className = command + 7 + entityIdStr.find_first_of(' ')+1;
		items_.emplace_back("Create C# Script Instance: " + className + " BindEntity: " + entityIdStr);
		CsharpVirtualEnvironmentOnQFE::GetInstance()->CreateScriptInstance(std::stoi(entityIdStr), className);
	} else if (strncmp(command, "cs_run ", 7) == 0) {
		std::string index = command + 7;
		std::string functionName = command + 9;
		items_.emplace_back("Run C# Script Function: " + functionName + " on Instance Index: " + index);
		CsharpVirtualEnvironmentOnQFE::GetInstance()->RunScriptFunction(std::stoi(index), functionName);
	} else if (strcmp(command, "cs_reload") == 0) {
		items_.emplace_back("Reload C# Assembly");
		CsharpVirtualEnvironmentOnQFE::GetInstance()->ReloadAssembly();
	} else {
		items_.emplace_back("Unknown command. Type 'help' for list.");
	}
}
