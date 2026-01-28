#pragma once
#include "IEvent.h"

namespace QFE {
	class DropFileEvent final : public IEvent {
	public:
		DropFileEvent() = delete;
		DropFileEvent(nlohmann::json& data);
		~DropFileEvent() override = default;
		/// <summary>
		/// ドロチE�Eされたファイルのイベント�E琁E
		/// </summary>
		/// <param name="wparam">WPARAM</param>
		/// <param name="lparam">LPARAM</param>
		void OnEvent(WPARAM wparam, LPARAM lparam) override;
		UINT GetEventType()  override;
	};
}
