#pragma once
#include "ScriptFunctionList.h"

#define NOMINMAX
#include <Windows.h>
#include <vector>

namespace QFE::SCRIPT {
	/// @brief Windows用のスクリプトインスタンスを表す構造体
	struct WindowsScriptInstance {
		HMODULE gameDllHandle;
		std::vector<QFE::SCRIPT::ScriptFunctionInfo> scripts;
	};
}