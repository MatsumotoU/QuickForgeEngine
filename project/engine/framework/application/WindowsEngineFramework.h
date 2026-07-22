#pragma once
#include "WindowsQuickForgeEngineSystems.h"

/// @file WindowsEngineFramework.h
/// @brief Windowsアプリケーション用のQuickForgeエンジンフレームワークを提供するヘッダーファイル。
namespace QFE::FRAMEWORK {
	/// @brief Windowsアプリケーション用のQuickForgeエンジンシステムを作成する関数。
	bool CreateWindowsQuickForgeEngineSystems(
		HINSTANCE hInstance,
		const std::string& mainWindowName,
		uint32_t mainWindowWidth,
		uint32_t mainWindowHeight,
		WindowsQuickForgeEngineSystems& outSystems);

	/// @brief OSメッセージを1件処理する。終了要求を受けた場合はfalseを返す。
	bool ProcessWindowsApplicationMessage();
	/// @brief 入力とフレーム計測を開始する。
	bool BeginWindowsEngineFrame(WindowsQuickForgeEngineSystems& systems);
	/// @brief シーン、入力、描画、フレーム計測の終了処理を行う。
	void EndWindowsEngineFrame(WindowsQuickForgeEngineSystems& systems);
	/// @brief 各エンジンシステムを依存関係の逆順で終了する。
	void ShutdownWindowsQuickForgeEngineSystems(WindowsQuickForgeEngineSystems& systems);
}
