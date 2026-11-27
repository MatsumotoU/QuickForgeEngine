#pragma once

#include <cstdint>

namespace QFE {
	class StaticComponentRegistry;
}

namespace QFE::FRAMEWORK {
	class WindowsQuickForgeEngineSystems;
	class WindowsEngineResources;

	// Systems / Resources のレイアウトが変わったため、旧 DLL と混在させない。
	// 共通システムを本体側で更新するため、旧 DLL による二重更新を防ぐ。
	inline constexpr uint32_t GameModuleApiVersion = 10;
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
