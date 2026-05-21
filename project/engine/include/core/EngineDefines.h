#pragma once
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/core/Profiler.h"
#include "engine/include/core/Error/ErrorList.h"

namespace QFE {
	// デバッグ用マクロ.最適化オフのときのみログを出力するマクロ. 最適化オンのときは何もしない.
#ifdef QFE_OPTIMIZE_OFF

// === デバッグ用マクロの定義 ===
/// @brief デバッグログを出力するマクロ.
#define QFE_LOG(...) DebugLog(__VA_ARGS__)
/// @brief スコーププロファイルを記録するマクロ.
#define QFE_PROFILE_SCOPE ScopeProfile scopeProfile##__LINE__(__FUNCTION__)
/// @brief デバッグブレークを行うマクロ.
#define QFE_DEBUG_BREAK() DebugBreak()
/// @brief デバッグブレークを行うマクロ.
#define QFE_REPORT_USER_ERROR(message, error) ReportUserError(message, error)
/// @brief システムエラーを報告してデバッグブレークするマクロ.
#define QFE_REPORT_SYSTEM_ERROR(message, error) ReportSystemError(message, error)

#else // === 最適化 ===
#if defined(_MSC_VER)

#define QFE_NOOP(...) __noop(__VA_ARGS__)
#else
#define QFE_NOOP(...) ((void)0)
#endif

#define QFE_LOG(...) QFE_NOOP(__VA_ARGS__)
#define QFE_PROFILE_SCOPE QFE_NOOP()
#define QFE_DEBUG_BREAK() QFE_NOOP()
#define QFE_REPORT_USER_ERROR(message, error) QFE_NOOP(message, error)
#define QFE_REPORT_SYSTEM_ERROR(message, error) QFE_NOOP(message, error)
#endif
}
