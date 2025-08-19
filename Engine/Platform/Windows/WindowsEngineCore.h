#pragma once
#include "Core/IEngineCore.h"

class WindowsEngineCore final : public IEngineCore {
public:
	WindowsEngineCore() = default;
	~WindowsEngineCore() override = default;
	void Initialize() override;
	void MainLoop() override;
	void Shutdown() override;

private:

};