#include "DropFileEvent.h"
#include "string/MyString.h"

#include "EngineDefines.h"

namespace QFE::WINDOW {

DropFileEvent::DropFileEvent(nlohmann::json& data):IEvent(data) {}

void DropFileEvent::OnEvent(WPARAM wparam, LPARAM) {
	QFE_LOG("Call DropFileEvent");

	// ドロップされたファイルのパスを取得
	HDROP hDrop = (HDROP)wparam;
	POINT position{};
	DragQueryPoint(hDrop, &position);
	UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < fileCount; ++i) {
		const UINT length = DragQueryFileW(hDrop, i, nullptr, 0);
		std::wstring filePath(static_cast<size_t>(length) + 1, L'\0');
		DragQueryFileW(hDrop, i, filePath.data(), length + 1);
		filePath.resize(length);
		// ファイルパスをイベントデータに格納
		if (!eventData_.contains("DroppedFiles")) eventData_["DroppedFiles"] = nlohmann::json::array();
		eventData_["DroppedFiles"].push_back({{"path", ConvertString(filePath)}, {"x", position.x}, {"y", position.y}});
		QFE_LOG("Drop File: " + ConvertString(filePath));
	}
	DragFinish(hDrop);
}

 UINT DropFileEvent::GetEventType() {
	return WM_DROPFILES;
}

}
