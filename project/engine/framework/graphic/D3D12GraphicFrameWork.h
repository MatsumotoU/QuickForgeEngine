#pragma once
#include "dx12/DirectXResourceFramework.h"
#include "dx12/DirectXRenderFramework.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct InstanceMetaCPU;
struct RaytracingVertexAttribute;

namespace QFE::ASSET {
	struct ModelData;
}

namespace QFE::GRAPHIC {
	class D3D12GraphicEngine;
}

namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> CreateGraphicEngine(HWND hwnd);
	/// @brief グラフィックエンジンで描画する前の処理を行う関数
	bool PreDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンで描画した後の処理を行う関数
	bool PostDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンを終了する関数
	bool ShutdownGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);

	/// @brief モデルデータをレイトレーシング用の連続UV・三角形・メタデータへ変換する。
	void BuildGlobalMeshBuffers(
		const std::unordered_map<std::string, QFE::ASSET::ModelData>& modelDataMap,
		const std::map<std::string, uint32_t>& textureGpuIndexMap,
		std::vector<RaytracingVertexAttribute>& outGlobalVertexAttributes,
		std::vector<uint32_t>& outGlobalTriIndices,
		std::unordered_map<std::string, InstanceMetaCPU>& outModelMeta);
}
