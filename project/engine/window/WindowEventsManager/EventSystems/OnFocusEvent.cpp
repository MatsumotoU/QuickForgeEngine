#include "OnFocusEvent.h"
#include "EngineDefines.h"

using namespace QFE;
OnFocusEvent::OnFocusEvent(nlohmann::json& data) :IEvent(data) {}

void OnFocusEvent::OnEvent(WPARAM, LPARAM) {
	QFE_LOG("Window Focused");
}

UINT OnFocusEvent::GetEventType() {
	return WM_SETFOCUS;
}


