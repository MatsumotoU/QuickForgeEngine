#pragma once
#include "window/EditorWindowManager.h"
#include "command/EditorCommandExecutor.h"
#include "command/EditorCommandList.h"

#include <imgui/imgui.h>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	/// @brief ゲームエディタ
	class GameEditor {
	public:
		void Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow);
		void Update();
		void Draw();
		void PostDraw();
	private:
		EditorWindowManager windowManager_;
		EditorCommandExecutor commandExecutor_;
		EditorCommandList commandList_;
	};
}
