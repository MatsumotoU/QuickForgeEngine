#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "IEditorWindow.h"

#include <imgui/imgui.h>
#include <unordered_map>

#define NOMINMAX
#include <Windows.h>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	class EditorCommandList;

	enum class EditorWindowType {
		Hierarchy,
		SceneViewer,
		Inspector,
		Profiler,
		Console,
		AssetBrowser,
		Logger,
	};

	/// @brief エディタのウィンドウ管理クラス
	class EditorWindowManager final{
	public:
		void Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow);
		void Update();
		void Draw(EditorCommandList& commandList);

		/// @brief 指定されたウィンドウタイプのウィンドウにフォーカスがあるかどうかを返す
		bool IsWindowFocused(EditorWindowType windowType);

	private:

		// エディタで表示できるウィンドウたち
		std::unordered_map<EditorWindowType, std::unique_ptr<IEditorWindow>> editorWindowsMap_;
		std::set<uint32_t> selectedEntities_;
		HWND mainWindow_;
		SCENE::SceneManager* sceneManager_;
	};
}
