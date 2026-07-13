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
	class LoadSceneCommand : public IEditorCommand {
	public:
		LoadSceneCommand(const std::string& scenePath, QFE::SCENE::SceneManager* sceneManager);

		/// @brief シーンをロードする。
		void Execute() override;
		/// @brief シーンのロードを取り消す。
		void Undo() override;

	private:
		QFE::SCENE::SceneManager* sceneManager_;
		std::string scenePath_;
		nlohmann::json previousSceneData_;
	};
}
