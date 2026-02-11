#pragma once
#include <Windows.h>
#include <nlohmann/json.hpp>

namespace QFE {
	/// @brief ウィンドウイベントシステムのインターフェース
	class IEvent {
	public:
		IEvent() = delete;
		IEvent(nlohmann::json& eventData);
		virtual ~IEvent() = default;

		/// @brief イベント発生時の処理
		virtual void OnEvent(WPARAM wparam, LPARAM lparam) = 0;
		/// @brief イベントタイプの取得
		virtual UINT GetEventType() = 0;
	protected:
		nlohmann::json& eventData_;
	};
}
