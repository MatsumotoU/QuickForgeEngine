#include "framework/application/GameModuleApi.h"

// ゲーム側が何もしなくても、フレームワークのシーン処理が動くことを確認する。
extern "C" __declspec(dllexport) uint32_t QFE_GetGameModuleApiVersion() {
	return QFE::FRAMEWORK::GameModuleApiVersion;
}

extern "C" __declspec(dllexport) bool QFE_RegisterGameComponents(QFE::StaticComponentRegistry&) {
	return true;
}

extern "C" __declspec(dllexport) void QFE_UpdateGame(
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems&,
	QFE::FRAMEWORK::WindowsEngineResources&, float) {
}
