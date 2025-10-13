#pragma once
#include "../IEvent.h"

class DropFileEvent final : public IEvent {
public:
	DropFileEvent() = delete;
	DropFileEvent(nlohmann::json& data);
	~DropFileEvent() override = default;
	/// <summary>
	/// ドロップされたファイルのイベント処理
	/// </summary>
	/// <param name="wparam">WPARAM</param>
	/// <param name="lparam">LPARAM</param>
	void OnEvent(WPARAM wparam, LPARAM lparam) override;
	UINT GetEventType()  override;
};