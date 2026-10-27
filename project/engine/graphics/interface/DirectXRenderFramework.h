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
	class IGraphicEngine;
}

/// @file DirectXRenderFramework.h
/// @brief DirectXレンダリングに関するグラフィックエンジンAPIを提供するヘッダーファイル
namespace QFE::GRAPHIC {
	/// @brief シェーダーペアを作成する関数
	bool CreateShaderPair(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const std::string& vsDirName, const std::string& psDirName,
		const std::string& vsFileName, const std::string& psFileName,
		QFE::GRAPHIC::ShaderPairHandle& outShaderPairHandle);
	/// @brief パイプラインステートオブジェクトを作成する関数
	bool CreateGraphicPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::ShaderPairHandle& shaderPairHandle,
		const QFE::GRAPHIC::RasterizerType& rasterizerType,
		const QFE::GRAPHIC::BlendMode& blendMode,
		const QFE::GRAPHIC::DepthStencilDescType& depthStencilDescType,
		QFE::GRAPHIC::PSOHandle& outPSOHandle);
	/// @brief 出力先のRTVフォーマットを指定してパイプラインステートオブジェクトを作成する関数
	bool CreateGraphicPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::ShaderPairHandle& shaderPairHandle,
		const QFE::GRAPHIC::RasterizerType& rasterizerType,
		const QFE::GRAPHIC::BlendMode& blendMode,
		const QFE::GRAPHIC::DepthStencilDescType& depthStencilDescType,
		DXGI_FORMAT renderTargetFormat,
		QFE::GRAPHIC::PSOHandle& outPSOHandle);

	/// @brief パイプラインステートオブジェクトのルートパラメータの型を取得する関数
	bool GetGraphicPSORootParameterTypeList(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::PSOHandle& psoHandle,
		std::vector<D3D12_ROOT_PARAMETER_TYPE>& outRootParameterTypeList);

	/// @brief コンピュートシェーダーをコンパイルし、パイプラインステートオブジェクトを作成する関数
	bool CreateComputePSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const std::string& csDirPath,
		const std::string& csFileName,
		QFE::GRAPHIC::ComputePSOHandle& outPSOHandle);

	/// @brief 1つのUAVを対象にコンピュートパイプラインを実行する関数
	/// @details Dispatch数はUAVの大きさとシェーダーのnumthreadsから算出する。
	///          この関数は結果のコピーやリソースの最終状態への遷移を行わない。
	bool DispatchComputePSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::ComputePSOHandle& psoHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& uavHandle);
	/// @brief 複数のCBV/SRV/UAVをバインドしてコンピュートパイプラインを実行する。
	/// @param workItemCountX シェーダーに処理させるX方向の要素数。
	bool DispatchComputePSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::ComputePSOHandle& psoHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const std::vector<QFE::GRAPHIC::ViewTypeFlags>& resourceViewTypes,
		uint32_t workItemCountX,
		uint32_t workItemCountY = 1,
		uint32_t workItemCountZ = 1);

	/// @brief レイトレーシングパイプラインステートオブジェクトを作成する関数
	bool CreateRayTracingPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::RTPSOHandle& outPSOHandle,
		const std::string& dirPath, const std::string& rgsFileName);

	/// @brief オフスクリーンレンダーターゲットを作成する関数
	bool CreateOffScreenRenderTarget(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
		uint32_t width, uint32_t height, DXGI_FORMAT format);
	/// @brief Editor用のシーンテクスチャを作成し、GUIで参照できるIDを返す。
	bool CreateEditorSceneTexture(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
		QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle,
		uintptr_t& outGuiTextureId,
		uint32_t width,
		uint32_t height);
	/// @brief ビューポートを作成する関数
	bool CreateViewport(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::ViewPortHandle& outViewportHandle,
		uint32_t width, uint32_t height);
	/// @brief シザリング矩形を作成する関数
	bool CreateScissorRect(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::ScissorRectHandle& outScissorRectHandle,
		int32_t left, int32_t top, int32_t right, int32_t bottom);

	/// @brief レンダーターゲットを設定する関数
	bool SetRenderTarget(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& depthStencilBufferHandle,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets);
	/// @brief 深度ステンシルを使用せずにレンダーターゲットを設定する関数
	bool SetRenderTarget(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets);

	/// @brief グラフィックパイプラインステートオブジェクトを使用して描画する関数
	bool DrawGraphicPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::PSOHandle& psoHandle,
		const QFE::GRAPHIC::ViewPortHandle& viewportHandle,
		const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
		uint32_t instanceCount = 1,
		uint32_t startInstanceLocation = 0);

	/// @brief グラフィックパイプラインステートオブジェクトを使用して描画する関数.ルートパラメータを直接指定するバージョン
	bool DrawGraphicPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::PSOHandle& psoHandle,
		const QFE::GRAPHIC::ViewPortHandle& viewportHandle,
		const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
		const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes);
	/// @brief グラフィックパイプラインステートオブジェクトを使用して描画する関数.ルートパラメータを直接指定するバージョン.インデックスバッファを指定するバージョン
	bool DrawGraphicPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
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
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
		QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
		const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources);
	/// @brief レイトレーシングパイプラインを使用して描画.レンダーターゲットを指定するバージョン
	bool DrawRayTracingPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
		QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
		const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		const QFE::GRAPHIC::DirectXResourceHandle& renderTargetResourceHandle);

	/// @brief レイトレーシングパイプラインを使用して描画.レンダーターゲットを指定するバージョン.インデックスバッファとUVバッファを指定するバージョン
	bool ShadowSpecularRayTracingPSO(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
		QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
		const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& uvBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& instanceMetaBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& firstTextureBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& skyBoxTextureHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
		QFE::GRAPHIC::RenderTargetHandle finalRenderTargetHandle);
}
