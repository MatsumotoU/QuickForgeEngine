#include "DropFileEvent.h"
#include "string/MyString.h"

#include "EngineDefines.h"

using namespace QFE;
DropFileEvent::DropFileEvent(nlohmann::json& data):IEvent(data) {}

void DropFileEvent::OnEvent(WPARAM wparam, LPARAM lparam) {
	lparam;
	QFE_LOG("Call DropFileEvent");

	// ドロップされたファイルのパスを取得
	HDROP hDrop = (HDROP)wparam;
	UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < fileCount; ++i) {
		wchar_t filePath[MAX_PATH];
		DragQueryFile(hDrop, i, filePath, MAX_PATH);
		// ファイルパスをイベントデータに格納
		eventData_["DropFilePath"] = ConvertString(filePath);
		QFE_LOG("Drop File: " + ConvertString(filePath));
	}
	DragFinish(hDrop);
}

 UINT DropFileEvent::GetEventType() {
	return WM_DROPFILES;
}
