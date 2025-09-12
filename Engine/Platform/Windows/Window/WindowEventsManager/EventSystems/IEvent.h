#pragma once
#include <Windows.h>
#include "Externals/nlohmann/json.hpp"

class IEvent {
public:
	IEvent() = delete;
	IEvent(nlohmann::json& eventData);
	virtual ~IEvent() = default;
	virtual void OnEvent(WPARAM wparam, LPARAM lparam) = 0;
	virtual UINT GetEventType() = 0;
protected:
	nlohmann::json& eventData_;
};