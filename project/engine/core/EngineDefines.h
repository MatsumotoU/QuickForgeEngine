#pragma once
#include "logger/MyDebugLog.h"
#include "error/ErrorList.h"
#include "design-patterns/component/Archive.h"

namespace QFE {
	// デバッグ用マクロ.最適化オフのときのみログを出力するマクロ. 最適化オンのときは何もしない.
#ifdef QFE_OPTIMIZE_OFF

// === デバッグ用マクロの定義 ===
/// @brief デバッグログを出力するマクロ.
#define QFE_LOG(...) QFE::DebugLog(__VA_ARGS__)
/// @brief スコーププロファイルを記録するマクロ.
#define QFE_PROFILE_SCOPE QFE::ScopeProfile scopeProfile##__LINE__(__FUNCTION__)
/// @brief デバッグブレークを行うマクロ.
#define QFE_DEBUG_BREAK() QFE::DebugBreak()
/// @brief デバッグブレークを行うマクロ.
#define QFE_REPORT_USER_ERROR(message, error) QFE::ReportUserError(message, error)
/// @brief システムエラーを報告してデバッグブレークするマクロ.
#define QFE_REPORT_SYSTEM_ERROR(message, error) QFE::ReportSystemError(message, error)
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

// リフレクション用マクロ
#define QFE_REFLECT_BEGIN(ClassName) \
    static std::string GetTypeName() { return #ClassName; } \
    void Reflect(QFE::Archive& ar) {

#define QFE_REFLECT_MEMBER(MemberName) \
        ar.Process(#MemberName, MemberName);

#define QFE_REFLECT_END() \
    }

// コンポーネントの自動登録用マクロ
/// @brief コンポーネントクラスを自動登録するマクロ. クラス定義の外で使用する必要があります.
#define QFE_COMPONENT(ClassName) \
    inline static QFE::ComponentRegisterer<ClassName> _autoReg##ClassName(#ClassName);

}
