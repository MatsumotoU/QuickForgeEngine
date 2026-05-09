#include "engine/include/utility/DebugTool/App/WinAppDebugCore.h"
#include "engine/include/utility/String/MyString.h"
#include "engine/include/core/EngineDefines.h"


#pragma comment(lib,"Dbghelp.lib")
using namespace QFE;

WinAppDebugCore::WinAppDebugCore(const LPSTR& lpCmdLine) {
	SetUnhandledExceptionFilter(ExportDump);
	lpCmdLine;
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("=====WinAppDebugCore=====");
#endif // QFE_OPTIMIZE_OFF

	// exe繧定ｵｷ蜍輔＠縺溘ヱ繧ｹ
	wchar_t fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH);
	std::string exeName(ConvertString(fileName));
	
#ifdef QFE_OPTIMIZE_OFF
	// 繧ｳ繝槭Φ繝牙ｼ墓焚遒ｺ隱・
	if (std::strcmp(lpCmdLine, "\0") != 0) {
		QFE_LOG("!!! EnebleCommandLineArguments !!!");
		QFE_LOG(std::format("EnebleCommand : {}", lpCmdLine));	
	} else {
		QFE_LOG("DisableCommandLineArguments");
	}
	QFE_LOG("");
#endif // QFE_OPTIMIZE_OFF
}

WinAppDebugCore::~WinAppDebugCore() {
}


