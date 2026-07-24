#include "WindowsScriptWorkFrame.h"
#include "script/ScriptInstance.h"
#include "design-patterns/EntityManager.h"
#include "design-patterns/component/DynamicComponent.h"

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
	const std::wstring& dllPath, const std::string& manifestFuncName,
	QFE::EntityManager* entityManager) {

	// インスタンス生成
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance = std::make_unique<QFE::SCRIPT::WindowsScriptInstance>();
	// DLLをロード
	scriptInstance->gameDllHandle = LoadDll(dllPath);
	if (scriptInstance->gameDllHandle == nullptr) {
		return nullptr;
	}
	// DLLからManifest関数のポインタを取得
	FARPROC getManifestFunc = GetProcAddress(scriptInstance->gameDllHandle, manifestFuncName.c_str());
	if (getManifestFunc != nullptr) {
		GetManifestFunc GetManifest = reinterpret_cast<GetManifestFunc>(getManifestFunc);
		QFE::SCRIPT::ScriptFunctionInfo* functionArray = nullptr;
		size_t functionCount = GetManifest(&functionArray);
		scriptInstance->scripts.reserve(functionCount);
		for (size_t i = 0; i < functionCount; ++i) {
			scriptInstance->scripts.push_back(functionArray[i]);
		}
	}

	if (entityManager != nullptr) {
		FARPROC componentManifestProc =
			GetProcAddress(scriptInstance->gameDllHandle, "QFE_GetComponentManifest");
		if (componentManifestProc != nullptr) {
			auto getComponentManifest =
				reinterpret_cast<QFE::GetPluginComponentManifestFunc>(componentManifestProc);
			QFE::PluginComponentManifest manifest{};
			if (getComponentManifest(&manifest) && manifest.apiVersion == 1) {
				for (size_t i = 0; i < manifest.componentCount; ++i) {
					if (entityManager->RegisterDynamicComponent(manifest.components[i])) {
						scriptInstance->registeredComponentNames.emplace_back(
							manifest.components[i].stableName);
					}
				}
			}
		}
	}

	return scriptInstance;
	
}

bool QFE::FRAMEWORK::UnloadWindowsScriptInstance(
	QFE::SCRIPT::WindowsScriptInstance* scriptInstance,
	QFE::EntityManager* entityManager) {
	if (scriptInstance == nullptr || scriptInstance->gameDllHandle == nullptr) {
		return true;
	}

	if (!scriptInstance->registeredComponentNames.empty()) {
		if (entityManager == nullptr) {
			QFE_LOG("EntityManager is required to unload a DLL with registered components.");
			return false;
		}
		// destroy/reflect関数がDLL内にあるため、必ずFreeLibraryより先に破棄する。
		while (!scriptInstance->registeredComponentNames.empty()) {
			const std::string& componentName =
				scriptInstance->registeredComponentNames.back();
			if (!entityManager->UnregisterDynamicComponent(componentName)) {
				QFE_LOG("Failed to unregister dynamic component: " + componentName);
				return false;
			}
			scriptInstance->registeredComponentNames.pop_back();
		}
	}

	scriptInstance->scripts.clear();
	if (!UnloadDll(scriptInstance->gameDllHandle)) {
		return false;
	}
	scriptInstance->gameDllHandle = nullptr;
	return true;
}
