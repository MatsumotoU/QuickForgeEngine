#pragma once
#include "IEvent.h"
namespace QFE {
	/// @brief ウィンドウがフォーカスを得たときのイベントシステム
	class OnFocusEvent final : public IEvent {
	public:
		OnFocusEvent() = delete;
		OnFocusEvent(nlohmann::json& data);
		~OnFocusEvent() override = default;
		OnFocusEvent(const OnFocusEvent&) = default;
		OnFocusEvent(OnFocusEvent&&) noexcept = default;
		OnFocusEvent& operator=(const OnFocusEvent&) = delete;
		OnFocusEvent& operator=(OnFocusEvent&&) = delete;
		
		/// @brief ウィンドウがフォーカスを得たときのイベント処理
		void OnEvent(WPARAM wparam, LPARAM lparam) override;
		UINT GetEventType() override;
	};
}
