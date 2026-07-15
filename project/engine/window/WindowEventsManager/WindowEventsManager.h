#pragma once
#include <Windows.h>
#include <array>
#include <memory>
#include "EventSystems/IEvent.h"

namespace QFE {
	/// @brief ウィンドウに対するアクションを管理するクラス
	class WindowEventsManager final {
	public:
		WindowEventsManager();
		~WindowEventsManager() = default;

		/// @brief ウィンドウプロシージャ(OS呼び出し)
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		/// @brief メッセージを処理して適切なイベントシステムに渡す
		LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	private:
		static inline constexpr int kEventSystemCount_ = 3;
		nlohmann::json eventData_;
		std::array<std::unique_ptr<IEvent>, kEventSystemCount_> eventSystems_;
	};
}
