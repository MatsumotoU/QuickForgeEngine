#pragma once
#include "../IEvent.h"
class ExitAppEvent final : public IEvent {
public:
	ExitAppEvent() = delete;
	ExitAppEvent(nlohmann::json& data);
	~ExitAppEvent() override = default;
	/// <summary>
	/// アプリケーションの終了イベント処理
	/// </summary>
	/// <param name="wparam">WPARAM</param>
	/// <param name="lparam">LPARAM</param>
	void OnEvent(WPARAM wparam, LPARAM lparam) override;
	UINT GetEventType() override;
};