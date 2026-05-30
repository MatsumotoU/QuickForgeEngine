#include "OnFocusEvent.h"
#include "EngineDefines.h"

using namespace QFE;
OnFocusEvent::OnFocusEvent(nlohmann::json& data) :IEvent(data) {}

void OnFocusEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; lparam;
	QFE_LOG("Whindow Focused");
}

UINT OnFocusEvent::GetEventType() {
	return WM_SETFOCUS;
}


