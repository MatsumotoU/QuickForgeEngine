#include "engine/include/window/windowEventsManager/EventSystems/OnFocusEvent.h"

#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/EngineDefines.h"

using namespace QFE;

OnFocusEvent::OnFocusEvent(nlohmann::json& data) :IEvent(data) {}

void OnFocusEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	wparam; lparam;
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("Whindow Focused");
#endif // QFE_OPTIMIZE_OFF
	
}

UINT OnFocusEvent::GetEventType() {
	return WM_SETFOCUS;
}


