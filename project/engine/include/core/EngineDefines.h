#pragma once
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/core/Profiler.h"

namespace QFE {
	// デバッグ用マクロ.最適化オフのときのみログを出力するマクロ. 最適化オンのときは何もしない.
#ifdef QFE_OPTIMIZE_OFF
#define QFE_LOG(...) DebugLog(__VA_ARGS__)
#define QFE_PROFILE_SCOPE ScopeProfile scopeProfile##__LINE__(__FUNCTION__)
#else
#define QFE_LOG(...) ((void)0)
#define QFE_PROFILE_SCOPE ((void)0)
#endif
}
