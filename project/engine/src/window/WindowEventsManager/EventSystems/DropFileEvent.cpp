#include "engine/include/window/windowEventsManager/EventSystems/DropFileEvent.h"
#include "engine/include/utility/String/MyString.h"
#ifdef _DEBUG
#include "engine/include/assets/AssetManager.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

DropFileEvent::DropFileEvent(nlohmann::json& data):IEvent(data) {}

void DropFileEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	lparam;
#ifdef _DEBUG
	DebugLog("Call DropFileEvent");
#endif // _DEBUG

	// 繝峨Ο繝・・縺輔ｌ縺溘ヵ繧｡繧､繝ｫ縺ｮ蜃ｦ逅・
	HDROP hDrop = (HDROP)wparam;
	UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < fileCount; ++i) {
		wchar_t filePath[MAX_PATH];
		DragQueryFile(hDrop, i, filePath, MAX_PATH);
		eventData_["DropFilePath"] = ConvertString(filePath);
#ifdef _DEBUG
		
		DebugLog("Drop File: " + ConvertString(filePath));
#endif // _DEBUG
	}
	DragFinish(hDrop);
}

 UINT DropFileEvent::GetEventType() {
	return WM_DROPFILES;
}
