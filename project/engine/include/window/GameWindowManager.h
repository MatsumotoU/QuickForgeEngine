#pragma once
#include <string>
#include <Windows.h>
#include "engine/include/core/Window/IGameWindowManager.h"
#include "engine/include/core/Window/IGameWindow.h"

class GameWindowManager final : public IGameWindowManager {
public:
	GameWindowManager();
	~GameWindowManager() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Shutdown() override;
	void AddWindow(const uint32_t& width, const uint32_t& height, const std::string& windowName) override;
	bool IsWindowActive() const override;

	HWND GetWindow(const std::string windowName) const;
};
