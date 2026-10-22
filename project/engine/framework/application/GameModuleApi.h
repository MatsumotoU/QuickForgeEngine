#pragma once

#include <cstdint>

namespace QFE {
	class StaticComponentRegistry;
}

namespace QFE::FRAMEWORK {
	class WindowsQuickForgeEngineSystems;
	class WindowsEngineResources;

	inline constexpr uint32_t GameModuleApiVersion = 1;
	inline constexpr const char* GetGameModuleApiVersionSymbol = "QFE_GetGameModuleApiVersion";
	inline constexpr const char* RegisterGameComponentsSymbol = "QFE_RegisterGameComponents";
	inline constexpr const char* UpdateGameSymbol = "QFE_UpdateGame";

	using GetGameModuleApiVersionFunction = uint32_t (*)();
	using RegisterGameComponentsFunction = bool (*)(StaticComponentRegistry& registry);
	using UpdateGameFunction = void (*)(
		WindowsQuickForgeEngineSystems& systems,
		WindowsEngineResources& resources,
		float deltaTime);
}
