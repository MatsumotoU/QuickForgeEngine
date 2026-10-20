#include "framework/application/WindowsGameModule.h"

#define NOMINMAX
#include <Windows.h>

#include <bit>

namespace {
	template <typename T>
	T ResolveFunction(HMODULE module, const char* symbol) {
		static_assert(sizeof(T) == sizeof(FARPROC));
		return std::bit_cast<T>(GetProcAddress(module, symbol));
	}
}

QFE::FRAMEWORK::WindowsGameModule::~WindowsGameModule() {
	Unload();
}

bool QFE::FRAMEWORK::WindowsGameModule::Load(const std::wstring& path) {
	if (nativeModule_ != nullptr) {
		return false;
	}

	HMODULE module = LoadLibraryW(path.c_str());
	if (module == nullptr) {
		return false;
	}

	const auto getApiVersion = ResolveFunction<GetGameModuleApiVersionFunction>(
		module, GetGameModuleApiVersionSymbol);
	const auto registerComponents = ResolveFunction<RegisterGameComponentsFunction>(
		module, RegisterGameComponentsSymbol);
	const auto updateGame = ResolveFunction<UpdateGameFunction>(module, UpdateGameSymbol);
	if (getApiVersion == nullptr || registerComponents == nullptr || updateGame == nullptr ||
		getApiVersion() != GameModuleApiVersion) {
		FreeLibrary(module);
		return false;
	}

	nativeModule_ = module;
	registerComponents_ = registerComponents;
	updateGame_ = updateGame;
	return true;
}

bool QFE::FRAMEWORK::WindowsGameModule::IsLoaded() const {
	return nativeModule_ != nullptr;
}

QFE::FRAMEWORK::RegisterGameComponentsFunction
QFE::FRAMEWORK::WindowsGameModule::GetComponentRegistrationFunction() const {
	return registerComponents_;
}

bool QFE::FRAMEWORK::WindowsGameModule::Update(
	WindowsQuickForgeEngineSystems& systems,
	WindowsEngineResources& resources,
	float deltaTime) const {
	if (updateGame_ == nullptr) {
		return false;
	}
	updateGame_(systems, resources, deltaTime);
	return true;
}

void QFE::FRAMEWORK::WindowsGameModule::Unload() {
	if (nativeModule_ != nullptr) {
		FreeLibrary(static_cast<HMODULE>(nativeModule_));
	}
	nativeModule_ = nullptr;
	registerComponents_ = nullptr;
	updateGame_ = nullptr;
}
