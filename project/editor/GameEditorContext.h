#pragma once
#include <imgui/imgui.h>
#include "scene/SceneManager.h"
#define NOMINMAX
#include <Windows.h>

#include <functional>

namespace QFE::EDITOR {
	/// @brief ゲームエディタのコンテキスト
	struct GameEditorContext {
		/// @brief シーンマネージャー
		QFE::SCENE::SceneManager* sceneManager;
		/// @brief シーンのテクスチャID
		ImTextureID sceneTextureId;
		/// @brief メインウィンドウのハンドル
		HWND mainWindow;

		std::function<void()> onSceneChanged;
	};
}