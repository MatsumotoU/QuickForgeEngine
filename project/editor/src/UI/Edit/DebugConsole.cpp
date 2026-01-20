/**
 * @file DebugConsole.cpp
 * @brief エディタ内コマンドラインコンソールの実装
 */

#include "editor/include/UI/Edit/DebugConsole.h"
#include "editor/include/Commands/AllEditorCommands.h"

void DebugConsole::Initialize() {
	name_ = "DebugConsole";
	isActive_ = false;

	commandList_.clear();
	commandList_.emplace_back(std::make_unique<EchoCommand>(items_,inputBuf_));
	commandList_.emplace_back(std::make_unique<ClearCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<LuaScriptInitializeCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<LuaScriptUpdateCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<LuaScriptReloadCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<SceneRunCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<SceneStopCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSCreateProjectCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSLoadAssemblyCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSOpenProjectCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSCompileScriptCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSCreateScriptInstanceCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSCreateBoundScriptInstanceCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSRunScriptCommand>(items_, inputBuf_));
	commandList_.emplace_back(std::make_unique<CSReloadAssemblyCommand>(items_, inputBuf_));
}

void DebugConsole::Update() {
	// 更新処理は必要ないため空実装としています
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
	if (strcmp(command, "help") == 0) {
		for (const auto& cmd : commandList_) {
			const auto aliases = cmd->GetAliases();
			std::string aliasList;
			for (size_t i = 0; i < aliases.size(); ++i) {
				aliasList += aliases[i];
				if (i < aliases.size() - 1) {
					aliasList += ", ";
				}
			}
			items_.emplace_back("Available command: " + aliasList);
		}

	} else {
		// 登録されているコマンドを検索して実行
		for (const auto& cmd : commandList_) {
			cmd->Execute();
		}
	}
}