#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "IEditorWindow.h"

#include <imgui/imgui.h>

#define NOMINMAX
#include <Windows.h>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	class EditorCommandList;
	

	/// @brief エディタのウィンドウ管理クラス
	class EditorWindowManager final{
	public:
		void Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow);
		void Update();
		void Draw(EditorCommandList& commandList);

	private:
		// エディタで表示できるウィンドウたち
		std::vector<std::unique_ptr<IEditorWindow>> editorWindows_;
		std::set<uint32_t> selectedEntities_;
		HWND mainWindow_;
		SCENE::SceneManager* sceneManager_;
	};
}
