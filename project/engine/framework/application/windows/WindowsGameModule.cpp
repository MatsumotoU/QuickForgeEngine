#include "framework/application/WindowsGameModule.h"

#define NOMINMAX
#include <Windows.h>

#include <array>
#include <atomic>
#include <bit>
#include <filesystem>
#include <system_error>
#include <utility>

#include "EngineDefines.h"
#include "string/MyString.h"

namespace {
	std::atomic_uint64_t gShadowCopySequence = 0;

	template <typename T>
	T ResolveFunction(HMODULE module, const char* symbol) {
		static_assert(sizeof(T) == sizeof(FARPROC));
		return std::bit_cast<T>(GetProcAddress(module, symbol));
	}

	std::filesystem::path MakeShadowCopyPath(const std::filesystem::path& sourcePath) {
		const uint64_t sequence = gShadowCopySequence.fetch_add(1, std::memory_order_relaxed);
		const std::wstring suffix =
			L".qfe-hot-" + std::to_wstring(GetCurrentProcessId()) + L"-" +
			std::to_wstring(sequence);
		return sourcePath.parent_path() /
			(sourcePath.stem().wstring() + suffix + sourcePath.extension().wstring());
	}

	std::filesystem::path GetExecutableDirectory() {
		std::array<wchar_t, 32768> executablePath{};
		const DWORD length = GetModuleFileNameW(
			nullptr, executablePath.data(), static_cast<DWORD>(executablePath.size()));
		if (length == 0 || length >= static_cast<DWORD>(executablePath.size())) {
			return {};
		}
		return std::filesystem::path(executablePath.data(), executablePath.data() + length)
			.parent_path();
	}

	std::filesystem::path ResolveModuleSourcePath(const std::filesystem::path& requestedPath) {
		std::error_code error;
		if (requestedPath.is_absolute()) {
			if (std::filesystem::is_regular_file(requestedPath, error) && !error) {
				return requestedPath;
			}
			return {};
		}

		const std::filesystem::path executableDirectory = GetExecutableDirectory();
		if (!executableDirectory.empty()) {
			const std::filesystem::path executableRelativePath =
				executableDirectory / requestedPath;
			if (std::filesystem::is_regular_file(executableRelativePath, error) && !error) {
				return executableRelativePath;
			}
		}

		error.clear();
		const std::filesystem::path workingDirectoryRelativePath =
			std::filesystem::absolute(requestedPath, error);
		if (!error &&
			std::filesystem::is_regular_file(workingDirectoryRelativePath, error) && !error) {
			return workingDirectoryRelativePath;
		}

		return {};
	}
}

QFE::FRAMEWORK::WindowsGameModule::~WindowsGameModule() {
	Unload();
}

bool QFE::FRAMEWORK::WindowsGameModule::Load(const std::wstring& path) {
	QFE_LOG("Loading game module: " + ConvertString(path));
	if (nativeModule_ != nullptr) {
		return false;
	}

	const std::filesystem::path sourcePath = ResolveModuleSourcePath(path);
	if (sourcePath.empty()) {
		QFE_LOG("Game module was not found next to the executable or in the working directory: " +
			ConvertString(path));
		return false;
	}

	std::error_code error;
	const std::filesystem::path loadedPath = MakeShadowCopyPath(sourcePath);
	if (!std::filesystem::copy_file(
		sourcePath, loadedPath, std::filesystem::copy_options::overwrite_existing, error)) {
		QFE_LOG(error.message());
		return false;
	}

	std::filesystem::path loadedPdbPath;
	const std::filesystem::path sourcePdbPath = sourcePath.parent_path() /
		(sourcePath.stem().wstring() + L".pdb");
	if (std::filesystem::is_regular_file(sourcePdbPath, error) && !error) {
		loadedPdbPath = loadedPath;
		loadedPdbPath.replace_extension(L".pdb");
		std::error_code pdbError;
		std::filesystem::copy_file(
			sourcePdbPath, loadedPdbPath,
			std::filesystem::copy_options::overwrite_existing, pdbError);
		if (pdbError) {
			loadedPdbPath.clear();
		}
	}

	HMODULE module = LoadLibraryW(loadedPath.c_str());
	if (module == nullptr) {
		std::filesystem::remove(loadedPath, error);
		if (!loadedPdbPath.empty()) {
			std::filesystem::remove(loadedPdbPath, error);
		}
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
		std::filesystem::remove(loadedPath, error);
		if (!loadedPdbPath.empty()) {
			std::filesystem::remove(loadedPdbPath, error);
		}
		return false;
	}

	nativeModule_ = module;
	registerComponents_ = registerComponents;
	updateGame_ = updateGame;
	sourcePath_ = sourcePath;
	loadedPath_ = loadedPath;
	loadedPdbPath_ = loadedPdbPath;
	return true;
}

bool QFE::FRAMEWORK::WindowsGameModule::IsLoaded() const {
	return nativeModule_ != nullptr;
}

const std::filesystem::path& QFE::FRAMEWORK::WindowsGameModule::GetSourcePath() const {
	return sourcePath_;
}

void QFE::FRAMEWORK::WindowsGameModule::Swap(WindowsGameModule& other) noexcept {
	using std::swap;
	swap(nativeModule_, other.nativeModule_);
	swap(registerComponents_, other.registerComponents_);
	swap(updateGame_, other.updateGame_);
	swap(sourcePath_, other.sourcePath_);
	swap(loadedPath_, other.loadedPath_);
	swap(loadedPdbPath_, other.loadedPdbPath_);
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

	std::error_code error;
	if (!loadedPath_.empty()) {
		std::filesystem::remove(loadedPath_, error);
	}
	if (!loadedPdbPath_.empty()) {
		std::filesystem::remove(loadedPdbPath_, error);
	}
	sourcePath_.clear();
	loadedPath_.clear();
	loadedPdbPath_.clear();
}
