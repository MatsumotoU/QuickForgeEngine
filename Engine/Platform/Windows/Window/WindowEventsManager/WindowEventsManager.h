#pragma once
#include <Windows.h>
#include <array>
#include <memory>
#include "EventSystems/IEvent.h"

class WindowEventsManager final {
public:
	WindowEventsManager();
	~WindowEventsManager() = default;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	static inline constexpr int kEventSystemCount_ = 2;
	nlohmann::json eventData_;
	std::array<std::unique_ptr<IEvent>, kEventSystemCount_> eventSystems_;
};