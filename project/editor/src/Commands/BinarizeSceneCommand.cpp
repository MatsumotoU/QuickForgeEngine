#include "editor/include/Commands/BinarizeSceneCommand.h"

#include "engine/include/scene/SceneManager.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

QFE::BinarizeSceneCommand::BinarizeSceneCommand(std::vector<std::string>& consoleLog, const char* command) 
	: IEditorCommand(consoleLog, command),aliases_({ "save_scene_binarize","ssb" }) {
}

void QFE::BinarizeSceneCommand::Execute() {
	if (!command_) return;
	for (auto& alias : aliases_) {
		if (strncmp(command_, alias.c_str(), alias.size()) == 0) {
			// シーンの保存場所を取得
			std::string message = command_ + alias.length();
			if (message.length() > 0 && message[0] == ' ') {
				message = message.substr(1);
			}
			cons_.emplace_back("Scene Save Name: " + message);

			// シーンの保存
			SceneManager::GetInstance()->SaveSceneBinary(message);
			return;
		}
	}
}

void QFE::BinarizeSceneCommand::Undo() {
	// Undoは特に意味がないため空実装
}

const std::vector<std::string> QFE::BinarizeSceneCommand::GetAliases() const {
	return aliases_;
}
