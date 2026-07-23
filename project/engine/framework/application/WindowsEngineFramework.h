#pragma once
#include "WindowsQuickForgeEngineSystems.h"
#include "WindowsEngineResources.h"


#include <string>
#include <unordered_map>
#include <map>

namespace QFE::ASSET {
	struct ModelData;
}

namespace QFE::GRAPHIC {
	enum class DirectXResourceHandle : uint32_t;
	enum class BLASHandle : uint32_t;
}

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

	/// @brief Windowsアプリケーション用のQuickForgeエンジンを初期化する関数。
	void EngineInitialize(WindowsQuickForgeEngineSystems& systems, WindowsEngineResources& resources);
	/// @brief Windowsアプリケーション用のQuickForgeエンジンの描画前処理を行う関数。
	void EnginePreDraw(WindowsQuickForgeEngineSystems& systems, WindowsEngineResources& resources);
	/// @brief Windowsアプリケーション用のQuickForgeエンジンの描画後処理を行う関数。
	void EnginePostDraw(WindowsQuickForgeEngineSystems& systems, WindowsEngineResources& resources);


	/// @brief 指定されたモデルファイルの頂点データを読み込み、モデルデータと頂点バッファを作成する。
	bool LoadModelVertexData(
		WindowsQuickForgeEngineSystems& systems,
		const std::string& modelDir, const std::string& modelName,
		std::unordered_map<std::string, QFE::ASSET::ModelData>& modelData,
		std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle>& vertexBuffers);
	/// @brief 指定されたモデルファイルのインデックスデータを読み込み、モデルデータとインデックスバッファを作成する。
	bool LoadModelIndexBuffer(
		WindowsQuickForgeEngineSystems& systems,
		const std::string& modelDir, const std::string& modelName,
		std::unordered_map<std::string, QFE::ASSET::ModelData>& modelData,
		std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle>& indexBuffers);
	/// @brief 指定されたモデルファイルの頂点データとインデックスデータを読み込み、モデルデータとBLASを作成する。
	bool LoadModelAndCreateBLAS(
		WindowsQuickForgeEngineSystems& systems,
		const std::string& modelDir, const std::string& modelName,
		std::unordered_map<std::string, QFE::ASSET::ModelData>& modelData,
		std::unordered_map<std::string, QFE::GRAPHIC::BLASHandle>& blasHandles);
	/// @brief 指定されたテクスチャファイルを読み込み、テクスチャハンドルを作成する。
	bool LoadTexture(
		WindowsQuickForgeEngineSystems& systems,
		const std::string& textureDir, const std::string& textureName,
		std::map<std::string, QFE::GRAPHIC::DirectXResourceHandle>& textureHandles,
		std::map<std::string, uint32_t>& textureGpuIndexMap,
		uint32_t& nextTextureGpuIndex);
}
