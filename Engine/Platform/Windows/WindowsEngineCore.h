#pragma once
#include "Core/IEngineCore.h"
#include <Windows.h>

#include "Window/GameWindowManager.h"

class WindowsEngineCore final : public IEngineCore {
public:
	WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine);
	~WindowsEngineCore() override = default;
	void Initialize() override;
	void MainLoop() override;
	void Shutdown() override;

private:
	HINSTANCE& hInstance_;
	LPSTR& lpCmdLine_;

	std::unique_ptr<IGameWindowManager> gameWindowManager;
};