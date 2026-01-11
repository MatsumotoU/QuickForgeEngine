#pragma once
#include "engine/include/core/IEngineCore.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "editor/include/IEditor.h"
#include <Windows.h>
#include <chrono>
#include <nlohmann/json.hpp>

// Core
#include "engine/include/window/GameWindowManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include "engine/include/utility/DebugTool/App/WinAppDebugCore.h"
#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#include "engine/include/utility/DebugTool/FrameCounter.h"
#include "engine/include/renderer/GraphRenderer.h"
#include "engine/include/graphic/PostEffect/OffScreen/OffScreenResourceManager.h"
#include "engine/include/graphic/PostEffect/RenderingPostprocess.h"
#include "engine/include/assets/AssetManager.h"

// なんかAssetsではなさそぁE�E��E�めE�E��E�めE
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"
#include "engine/include/assets/Script/CsharpVirtualEnvironmentOnQFE.h"

// サブモジュール
#include "engine/include/audio/AudioInterface.h"
#include "engine/include/input/InputInterface.h"
#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/collider/ColliderManager.h"
#include "engine/include/utility/MultiThreadTaskExecutor.h"

class WindowsEngineCore final : public IEngineCore {
public:
	WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine);
	~WindowsEngineCore() override = default;
	void Initialize() override;
	void MainLoop() override;
	void Shutdown() override;

private:
	void Update();
	void Draw();

	HINSTANCE& hInstance_;
	LPSTR& lpCmdLine_;

	std::unique_ptr<IGameWindowManager> gameWindowManager;
	DirectXCommon* directXCommon_;
	WinAppDebugCore debugCore_;

	OffScreenResourceManager offScreenResourceManager_;

	AssetManager* assetManager_;
	RenderingPostprocess* renderingPostprocess_;
	GraphRenderer* graphRenderer_;

	GraphicPipelineManager* graphicPipelineManager_;
	ImGuiFlameController imguiFrameController_;

	std::unique_ptr<IEditor> editor_ = nullptr;
	FrameCounter frameCounter_;

	InputInterface* inputInterface_;
	SceneManager* sceneManager_;
	LuaScriptResourceManager* luaScriptResourceManager_;
	CsharpVirtualEnvironmentOnQFE* csScriptManager_;
	PhysicsManager* physicsManager_;
	ColliderManager* colliderManager_;
	IAudioInterface* audioInterface_;

	MultiThreadTaskExecutor* multiThreadTaskExecutor_;
	nlohmann::json configJson_;
};
