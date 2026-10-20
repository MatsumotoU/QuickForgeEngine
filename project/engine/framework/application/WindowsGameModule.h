#pragma once

#include "GameModuleApi.h"

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
	};
}
