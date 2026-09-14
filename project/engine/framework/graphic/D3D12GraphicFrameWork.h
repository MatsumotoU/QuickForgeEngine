#pragma once
#include "dx12/DirectXResourceFramework.h"
#include "dx12/DirectXRenderFramework.h"
#include <map>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct InstanceMetaCPU;
struct RaytracingVertexAttribute;

namespace QFE::ASSET {
	struct ModelData;
}

namespace QFE::FRAMEWORK {
	class GraphicContext;

	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<GraphicContext> CreateGraphicEngine(HWND hwnd);
	/// @brief グラフィックエンジンで描画する前の処理を行う関数
	bool PreDrawGraphicEngine(GraphicContext* graphicEngine);
	/// @brief グラフィックエンジンで描画した後の処理を行う関数
	bool PostDrawGraphicEngine(GraphicContext* graphicEngine);
	/// @brief グラフィックエンジンを終了する関数
	bool ShutdownGraphicEngine(GraphicContext* graphicEngine);

	/// @brief Editor用のシーンテクスチャを作成し、ImGuiで参照できるIDを返す。
	bool CreateEditorSceneTexture(
		GraphicContext* graphicEngine,
		QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
		QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle,
		uintptr_t& outGuiTextureId,
		uint32_t width,
		uint32_t height);

	/// @brief モデルデータをレイトレーシング用の連続UV・三角形・メタデータへ変換する。
	void BuildGlobalMeshBuffers(
		const std::unordered_map<std::string, QFE::ASSET::ModelData>& modelDataMap,
		const std::map<std::string, uint32_t>& textureGpuIndexMap,
		std::vector<RaytracingVertexAttribute>& outGlobalVertexAttributes,
		std::vector<uint32_t>& outGlobalTriIndices,
		std::unordered_map<std::string, InstanceMetaCPU>& outModelMeta);
}
