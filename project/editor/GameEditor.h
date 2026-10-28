#pragma once
#include "window/EditorWindowManager.h"
#include "command/EditorCommandExecutor.h"
#include "command/EditorCommandList.h"
#include "inspector/ComponentInspectorRegistry.h"

#include "framework/application/WindowsEngineFramework.h"

#include <imgui/imgui.h>

namespace QFE::SCENE {
	class SceneManager;
}
namespace QFE::WINDOW {
	class IGameWindowManager;
}

namespace QFE::EDITOR {
	enum class EditorCameraType {
		DebugCamera,
		MainCamera
	};

	/// @brief ゲームエディタ
	class GameEditor {
	public:
		void Initialize(
			QFE::SCENE::SceneManager* sceneManager,
			ImTextureID sceneTextureId,
			QFE::WINDOW::IGameWindowManager& windowManager,
			const QFE::FRAMEWORK::EngineConfig& config);
		void Update(QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,QFE::FRAMEWORK::WindowsEngineResources& resources);
		void Draw();
		void PostDraw();
		bool ConsumeGameModuleBuildAndReloadRequest();

		EditorCameraType GetActiveCameraType() const { return activeCameraType_; }
		ComponentInspectorRegistry& GetComponentInspectorRegistry() { return componentInspectorRegistry_; }

	private:
		// WindowManagerが保持するInspectorより長く生存させる。
		ComponentInspectorRegistry componentInspectorRegistry_;
		EditorWindowManager windowManager_;
		EditorCommandExecutor commandExecutor_;
		EditorCommandList commandList_;
		EditorCameraType activeCameraType_;
	};
}
