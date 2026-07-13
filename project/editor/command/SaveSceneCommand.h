#pragma once
#include "IEditorCommand.h"
#include <cstdint>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	/// @brief シーンのロードを行うコマンド
	class SaveSceneCommand : public IEditorCommand {
	public:
		SaveSceneCommand(const std::string& savePath, QFE::SCENE::SceneManager* sceneManager);

		/// @brief シーンを保存する。
		void Execute() override;
		/// @brief シーンの保存を取り消す。
		void Undo() override;

	private:
		QFE::SCENE::SceneManager* sceneManager_;
		std::string scenePath_;
		nlohmann::json previousSceneData_;
	};
}
