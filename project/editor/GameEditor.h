#pragma once
#include "window/EditorWindowManager.h"
#include "command/EditorCommandExecutor.h"
#include "command/EditorCommandList.h"

#include "framework/application/WindowsEngineFramework.h"

#include <imgui/imgui.h>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::EDITOR {
	enum class EditorCameraType {
		DebugCamera,
		MainCamera
	};

	/// @brief ゲームエディタ
	class GameEditor {
	public:
		void Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow);
		void Update(QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,QFE::FRAMEWORK::WindowsEngineResources& resources);
		void Draw();
		void PostDraw();

		EditorCameraType GetActiveCameraType() const { return activeCameraType_; }


	private:
		EditorWindowManager windowManager_;
		EditorCommandExecutor commandExecutor_;
		EditorCommandList commandList_;
		EditorCameraType activeCameraType_;
	};
}
