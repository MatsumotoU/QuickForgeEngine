#pragma once
#include "ScriptFunctionList.h"

#define NOMINMAX
#include <Windows.h>
#include <vector>
#include <string>

namespace QFE::SCRIPT {
	/// @brief Windows用のスクリプトインスタンスを表す構造体
	struct WindowsScriptInstance {
		HMODULE gameDllHandle = nullptr;
		std::vector<QFE::SCRIPT::ScriptFunctionInfo> scripts;
		std::vector<std::string> registeredComponentNames;
	};
}
