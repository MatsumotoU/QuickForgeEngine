#include "WindowsScriptWorkFrame.h"
#include "script/ScriptInstance.h"

HMODULE QFE::FRAMEWORK::LoadDll(const std::wstring& dllPath) {
	HMODULE dllHandle = LoadLibraryW(dllPath.c_str());
	if (dllHandle == nullptr) {
		QFE_LOG("Failed to load DLL: " + std::string(dllPath.begin(), dllPath.end()));
	}
	return dllHandle;
}

bool QFE::FRAMEWORK::UnloadDll(HMODULE dllHandle) {
	if (dllHandle != nullptr) {
		if (FreeLibrary(dllHandle)) {
			return true;
		} else {
			QFE_LOG("Failed to unload DLL.");
			return false;
		}
	}
	return false;
}

std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> QFE::FRAMEWORK::LoadWindowsScriptInstance(
	const std::wstring& dllPath, const std::string& manifestFuncName) {

	// インスタンス生成
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance = std::make_unique<QFE::SCRIPT::WindowsScriptInstance>();
	// DLLをロード
	scriptInstance->gameDllHandle = LoadDll(dllPath);
	assert(scriptInstance->gameDllHandle != nullptr);
	// DLLからManifest関数のポインタを取得
	FARPROC getManifestFunc = GetProcAddress(scriptInstance->gameDllHandle, manifestFuncName.c_str());
	assert(getManifestFunc != nullptr);
	// Manifest関数のポインタを関数型にキャスト
	GetManifestFunc GetManifest = reinterpret_cast<GetManifestFunc>(getManifestFunc);
	assert(GetManifest != nullptr);

	QFE::SCRIPT::ScriptFunctionInfo* functionArray = nullptr;
	// DLLから「ポインタ」と「個数」を受け取る
	size_t functionCount = GetManifest(&functionArray);

	// 安全にEXE側の管理するデータに詰め替える
	scriptInstance->scripts.reserve(functionCount);
	for (size_t i = 0; i < functionCount; ++i) {
		scriptInstance->scripts.push_back(functionArray[i]);
	}

	return scriptInstance;
	
}

void QFE::FRAMEWORK::UnloadWindowsScriptInstance(QFE::SCRIPT::WindowsScriptInstance* scriptInstance) {
	if (scriptInstance) {
		UnloadDll(scriptInstance->gameDllHandle);
		scriptInstance->scripts.clear();
	}
}
