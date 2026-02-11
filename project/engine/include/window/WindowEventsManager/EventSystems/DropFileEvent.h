#pragma once
#include "IEvent.h"

namespace QFE {
	/// @brief ファイルがウィンドウにドロップされたときのイベントシステム
	class DropFileEvent final : public IEvent {
	public:
		DropFileEvent() = delete;
		DropFileEvent(nlohmann::json& data);
		~DropFileEvent() override = default;

		/// @brief ファイルがウィンドウにドロップされたときのイベント処理
		void OnEvent(WPARAM wparam, LPARAM lparam) override;
		UINT GetEventType()  override;
	};
}
