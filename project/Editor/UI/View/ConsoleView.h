#pragma once
#include "../IEditorUI.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

class ConsoleView : public IEditorUI {
public:
	ConsoleView();
	~ConsoleView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
private:
#ifdef _DEBUG
	LogLevel logLevel_;
#endif // _DEBUG
};