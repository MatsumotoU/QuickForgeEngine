#pragma once
#include "IEditorCommand.h"
#include <cstdint>

namespace QFE::SCENE { class SceneManager; }

namespace QFE::EDITOR {
	/// 平行光エンティティの作成と取り消し。
	class CreateDirectionalLightCommand final : public IEditorCommand {
	public:
		explicit CreateDirectionalLightCommand(QFE::SCENE::SceneManager* sceneManager);
		void Execute() override;
		void Undo() override;

	private:
		QFE::SCENE::SceneManager* sceneManager_;
		uint32_t entityId_;
	};
}
