#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
class IGameWindow;

class IGameWindowManager {
public:
	virtual ~IGameWindowManager() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Shutdown() = 0;
	virtual void AddWindow(const uint32_t& width, const uint32_t& height, const std::string& windowName) = 0;
	virtual bool IsWindowActive() const = 0;

protected:
	std::vector<std::unique_ptr<IGameWindow>> windows;
};