#pragma once

#include "GameModuleApi.h"

#include <filesystem>
#include <string>

namespace QFE::FRAMEWORK {
	/// @brief Windows DLLとして提供されるゲームモジュールを保持する。
	class WindowsGameModule final {
	public:
		WindowsGameModule() = default;
		~WindowsGameModule();

		WindowsGameModule(const WindowsGameModule&) = delete;
		WindowsGameModule& operator=(const WindowsGameModule&) = delete;

		bool Load(const std::wstring& path);
		bool IsLoaded() const;
		const std::filesystem::path& GetSourcePath() const;
		void Swap(WindowsGameModule& other) noexcept;
		RegisterGameComponentsFunction GetComponentRegistrationFunction() const;
		bool Update(
			WindowsQuickForgeEngineSystems& systems,
			WindowsEngineResources& resources,
			float deltaTime) const;

	private:
		void Unload();

		void* nativeModule_ = nullptr;
		RegisterGameComponentsFunction registerComponents_ = nullptr;
		UpdateGameFunction updateGame_ = nullptr;
		std::filesystem::path sourcePath_;
		std::filesystem::path loadedPath_;
		std::filesystem::path loadedPdbPath_;
	};
}
