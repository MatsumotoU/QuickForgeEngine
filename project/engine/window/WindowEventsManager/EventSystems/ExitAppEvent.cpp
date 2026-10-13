#include "ExitAppEvent.h"
#include "EngineDefines.h"

namespace QFE::WINDOW {

ExitAppEvent::ExitAppEvent(nlohmann::json& data):IEvent(data) {}

void ExitAppEvent::OnEvent(WPARAM, LPARAM) {
	QFE_LOG("Call ExitAppEvent");

	// アプリケーション終了フラグを立てる
	eventData_["DestroyWindow"] = true;
}

UINT ExitAppEvent::GetEventType() {
	return WM_DESTROY;
}

}
