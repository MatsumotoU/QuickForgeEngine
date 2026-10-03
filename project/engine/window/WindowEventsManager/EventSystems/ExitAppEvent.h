#pragma once
#include "IEvent.h"

namespace QFE::WINDOW {
	/// @brief アプリケーション終了時のイベントシステム
	class ExitAppEvent final : public IEvent {
	public:
		ExitAppEvent() = delete;
		ExitAppEvent(nlohmann::json& data);
		~ExitAppEvent() override = default;
		/// @brief アプリケーション終了時のイベント処理
		void OnEvent(WPARAM wparam, LPARAM lparam) override;
		UINT GetEventType() override;
	};
}
