#pragma once
#include "IEvent.h"

class DropFileEvent final : public IEvent {
public:
	DropFileEvent() = delete;
	DropFileEvent(nlohmann::json& data);
	~DropFileEvent() override = default;
	/// <summary>
	/// 繝峨Ο繝・・縺輔ｌ縺溘ヵ繧｡繧､繝ｫ縺ｮ繧､繝吶Φ繝亥・逅・
	/// </summary>
	/// <param name="wparam">WPARAM</param>
	/// <param name="lparam">LPARAM</param>
	void OnEvent(WPARAM wparam, LPARAM lparam) override;
	UINT GetEventType()  override;
};
