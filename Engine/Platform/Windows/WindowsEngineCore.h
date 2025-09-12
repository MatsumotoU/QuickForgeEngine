#pragma once
#include "Core/IEngineCore.h"
#include <Windows.h>

#include "Window/GameWindowManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "Graphic/Pipeline/GraphicPipelineManager.h"
#include "AppUtility/DebugTool/App/WinAppDebugCore.h"
#include "AppUtility/DebugTool/ImGui/FrameController/ImGuiFlameController.h"

#include "Assets/AssetManager.h"

#include "Graphic/PostEffect/OffScreen/OffScreenResourceManager.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"

// TODO: AssetManagerに統合
#include "Platform/Windows/Assets/2DTexture/TextureManager.h"

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
	DirectXCommon directXCommon_;
	WinAppDebugCore debugCore_;

	OffScreenResourceManager offScreenResourceManager_;

	AssetManager assetManager_;
	RendaringPostprosecess rendaringPostprocess_;

	GraphicPipelineManager graphicPipelineManager_;
	ImGuiFlameController imguiFrameController_;
};