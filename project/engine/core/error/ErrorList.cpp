#include "ErrorList.h"
#include "../EngineDefines.h"

void QFE::DebugBreak() {
	// コンパイラ認識
#if defined(_MSC_VER)// Visual Studio (MSVC)用
	__debugbreak();
#else
	QFE_LOG("Debug break is not supported on this compiler.");
#endif
}

void QFE::ReportUserError(const std::string& message, UserError error)
{
	error; message;
	QFE_LOG("User Error: " + message, LogLevel::Error);
}

void QFE::ReportSystemError(const std::string& message, SystemError error){
	error; message;
	QFE_LOG("System Error: " + message);
	QFE_DEBUG_BREAK();
	throw std::runtime_error(message);
}
