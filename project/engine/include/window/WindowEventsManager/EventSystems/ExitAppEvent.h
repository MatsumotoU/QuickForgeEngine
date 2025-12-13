#pragma once
#include "IEvent.h"
class ExitAppEvent final : public IEvent {
public:
	ExitAppEvent() = delete;
	ExitAppEvent(nlohmann::json& data);
	~ExitAppEvent() override = default;
	/// <summary>
	/// アプリケーションの終亁E��ベント�E琁E
	/// </summary>
	/// <param name="wparam">WPARAM</param>
	/// <param name="lparam">LPARAM</param>
	void OnEvent(WPARAM wparam, LPARAM lparam) override;
	UINT GetEventType() override;
};
