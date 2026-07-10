#include "WindowsScriptWorkFrame.h"
#include "script/ScriptInstance.h"

#define NOMINMAX
#include <Windows.h>

std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> QFE::FRAMEWORK::CreateWindowsScriptInstance(const std::wstring& dllPath) {
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance = std::make_unique<QFE::SCRIPT::WindowsScriptInstance>();

	scriptInstance->gameDllHandle = LoadLibraryW(dllPath.c_str());
	assert(scriptInstance->gameDllHandle != nullptr);

	FARPROC getManifestFunc = GetProcAddress(scriptInstance->gameDllHandle, "GetManifest");
	assert(getManifestFunc != nullptr);

	GetManifestFunc GetManifest = reinterpret_cast<GetManifestFunc>(getManifestFunc);
	assert(GetManifest != nullptr);

	QFE::SCRIPT::ScriptFunctionInfo* functionArray = nullptr;
	// DLLから「ポインタ」と「個数」を受け取る
	size_t functionCount = GetManifest(&functionArray);

	// 安全にEXE側の管理するデータに詰め替える（これでDLLをアンロードしても安全！）
	scriptInstance->scripts.reserve(functionCount);
	for (size_t i = 0; i < functionCount; ++i) {
		scriptInstance->scripts.push_back(functionArray[i]);
	}

	return scriptInstance;
}

void QFE::FRAMEWORK::UnloadWindowsScriptInstance(QFE::SCRIPT::WindowsScriptInstance* scriptInstance) {
	if (scriptInstance) {
		if (scriptInstance->gameDllHandle) {
			FreeLibrary(scriptInstance->gameDllHandle);
			scriptInstance->gameDllHandle = nullptr;
		}
		delete scriptInstance;
	}
}
