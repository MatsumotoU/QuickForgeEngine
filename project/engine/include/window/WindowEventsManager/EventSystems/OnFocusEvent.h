#pragma once
#include "IEvent.h"
class OnFocusEvent final : public IEvent {
public:
	OnFocusEvent() = delete;
	OnFocusEvent(nlohmann::json& data);
	~OnFocusEvent() override = default;
	/// <summary>
	/// アプリケーションのフォーカスイベント処理
	/// </summary>
	/// <param name="wparam">WPARAM</param>
	/// <param name="lparam">LPARAM</param>
	void OnEvent(WPARAM wparam, LPARAM lparam) override;
	UINT GetEventType() override;
};
