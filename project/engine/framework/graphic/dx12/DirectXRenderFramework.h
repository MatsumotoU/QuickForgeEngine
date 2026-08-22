#pragma once
#include "math/MathInclude.h"
#include "graphics/dx12/GraphicEngineHandleTypes.h"
#include "graphics/dx12/pipeline/pso/PipelineDescTypes.h"
#include "graphics/dx12/vram/descriptors/ViewTypeFlags.h"

#include "graphics/dx12/pipeline/rtpso/TLAS.h"

#define NOMINMAX
#include <Windows.h>
#include <d3d12.h>
#include <memory>
#include <string>
#include <vector>
#include <utility>

struct VertexData;

namespace QFE::GRAPHIC {
	class D3D12GraphicEngine;
}

/// @file DirectXRenderFramework.h
/// @brief DirectXレンダリングに関するフレームワーク関数を提供するヘッダーファイル
namespace QFE::FRAMEWORK {
	/// @brief シェーダーペアを作成する関数
	bool CreateShaderPair(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::string& vsDirName, const std::string& psDirName,
		const std::string& vsFileName, const std::string& psFileName,
		QFE::GRAPHIC::ShaderPairHandle& outShaderPairHandle);
	/// @brief パイプラインステートオブジェクトを作成する関数
	bool CreateGraphicPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::ShaderPairHandle& shaderPairHandle,
		const QFE::GRAPHIC::RasterizerType& rasterizerType,
		const QFE::GRAPHIC::BlendMode& blendMode,
		const QFE::GRAPHIC::DepthStencilDescType& depthStencilDescType,
		QFE::GRAPHIC::PSOHandle& outPSOHandle);
	/// @brief 出力先のRTVフォーマットを指定してパイプラインステートオブジェクトを作成する関数
	bool CreateGraphicPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::ShaderPairHandle& shaderPairHandle,
		const QFE::GRAPHIC::RasterizerType& rasterizerType,
		const QFE::GRAPHIC::BlendMode& blendMode,
		const QFE::GRAPHIC::DepthStencilDescType& depthStencilDescType,
		DXGI_FORMAT renderTargetFormat,
		QFE::GRAPHIC::PSOHandle& outPSOHandle);

	/// @brief パイプラインステートオブジェクトのルートパラメータの型を取得する関数
	bool GetGraphicPSORootParameterTypeList(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::PSOHandle& psoHandle,
		std::vector<D3D12_ROOT_PARAMETER_TYPE>& outRootParameterTypeList);

	/// @brief レイトレーシングパイプラインステートオブジェクトを作成する関数
	bool CreateRayTracingPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::RTPSOHandle& outPSOHandle,
		const std::string& dirPath, const std::string& rgsFileName);

	/// @brief オフスクリーンレンダーターゲットを作成する関数
	bool CreateOffScreenRenderTarget(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
		uint32_t width, uint32_t height, DXGI_FORMAT format);
	/// @brief ビューポートを作成する関数
	bool CreateViewport(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::ViewPortHandle& outViewportHandle,
		uint32_t width, uint32_t height);
	/// @brief シザリング矩形を作成する関数
	bool CreateScissorRect(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::ScissorRectHandle& outScissorRectHandle,
		int32_t left, int32_t top, int32_t right, int32_t bottom);

	/// @brief レンダーターゲットを設定する関数
	bool SetRenderTarget(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& depthStencilBufferHandle,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets);

	/// @brief グラフィックパイプラインステートオブジェクトを使用して描画する関数
	bool DrawGraphicPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::PSOHandle& psoHandle,
		const QFE::GRAPHIC::ViewPortHandle& viewportHandle,
		const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets);

	/// @brief グラフィックパイプラインステートオブジェクトを使用して描画する関数.ルートパラメータを直接指定するバージョン
	bool DrawGraphicPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::PSOHandle& psoHandle,
		const QFE::GRAPHIC::ViewPortHandle& viewportHandle,
		const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
		const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes);
	/// @brief グラフィックパイプラインステートオブジェクトを使用して描画する関数.ルートパラメータを直接指定するバージョン.インデックスバッファを指定するバージョン
	bool DrawGraphicPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::PSOHandle& psoHandle,
		const QFE::GRAPHIC::ViewPortHandle& viewportHandle,
		const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
		const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes);

	/// @brief レイトレーシングパイプラインを使用して描画
	bool DrawRayTracingPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
		QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
		const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources);
	/// @brief レイトレーシングパイプラインを使用して描画.レンダーターゲットを指定するバージョン
	bool DrawRayTracingPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
		QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
		const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const QFE::GRAPHIC::DirectXResourceHandle& renderTargetResourceHandle);

	/// @brief レイトレーシングパイプラインを使用して描画.レンダーターゲットを指定するバージョン.インデックスバッファとUVバッファを指定するバージョン
	bool ShadowSpecularRayTracingPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
		QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
		const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& uvBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& instanceMetaBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& firstTextureBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		QFE::GRAPHIC::RenderTargetHandle finalRenderTargetHandle);
}
