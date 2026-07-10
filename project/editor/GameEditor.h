#pragma once
#include "window/EditorWindowManager.h"
#include "command/EditorCommandExecutor.h"
#include "command/EditorCommandList.h"

#include <imgui/imgui.h>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief ゲームエディタ
	class GameEditor {
	public:
		void Initialize(QFE::EntityManager* entityManager, ImTextureID sceneTextureId);
		void Update();
		void Draw();
		void PostDraw();
	private:
		EditorWindowManager windowManager_;
		EditorCommandExecutor commandExecutor_;
		EditorCommandList commandList_;
	};
}
