#include "OnFocusEvent.h"
#include "EngineDefines.h"

namespace QFE::WINDOW {

OnFocusEvent::OnFocusEvent(nlohmann::json& data) :IEvent(data) {}

void OnFocusEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; lparam;
	QFE_LOG("Window Focused");
}

UINT OnFocusEvent::GetEventType() {
	return WM_SETFOCUS;
}

}

