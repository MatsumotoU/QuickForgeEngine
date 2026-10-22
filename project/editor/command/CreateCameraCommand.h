#pragma once

#include "IEditorCommand.h"

#include <cstdint>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	/// @brief カメラエンティティを作成するコマンド。
	class CreateCameraCommand final : public IEditorCommand {
	public:
		explicit CreateCameraCommand(QFE::SCENE::SceneManager* sceneManager);

		/// @brief カメラエンティティを作成する。
		void Execute() override;
		/// @brief 作成したカメラエンティティを削除する。
		void Undo() override;

	private:
		QFE::SCENE::SceneManager* sceneManager_;
		uint32_t entityId_;
	};
}
