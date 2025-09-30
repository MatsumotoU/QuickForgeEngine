#pragma once
#include "Core/IEngineCore.h"
#include "Core/EngineGlobalValue.h"
#include "Editor/IEditor.h"
#include <Windows.h>
#include <chrono>
#include <nlohmann/json.hpp>

#include "Window/GameWindowManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "Graphic/Pipeline/GraphicPipelineManager.h"
#include "AppUtility/DebugTool/App/WinAppDebugCore.h"
#include "AppUtility/DebugTool/ImGui/FrameController/ImGuiFlameController.h"
#include "AppUtility/DebugTool/FrameCounter.h"
#include "Renderer/GraphRenderer.h"

#include "Graphic/PostEffect/OffScreen/OffScreenResourceManager.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"
#include "Input/DirectInput/DirectInputManager.h"

#include "Assets/AssetManager.h"
// なんかAssetsではなさそうなやつら
#include "Scene/SceneManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Physics/PhysicsManager.h"

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
	RendaringPostprosecess* rendaringPostprocess_;
	GraphRenderer* graphRenderer_;

	GraphicPipelineManager* graphicPipelineManager_;
	ImGuiFlameController imguiFrameController_;

	std::unique_ptr<IEditor> editor_ = nullptr;
	FrameCounter frameCounter_;

	DirectInputManager* directInputManager_;
	SceneManager* sceneManager_;
	LuaScriptResourceManager* luaScriptResourceManager_;
	PhysicsManager* physicsManager_;

	nlohmann::json configJson_;
};