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

namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> CreateGraphicEngine(HWND hwnd);
	/// @brief グラフィックエンジンで描画する前の処理を行う関数
	bool PreDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンで描画した後の処理を行う関数
	bool PostDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンを終了する関数
	bool ShutdownGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);

	/// @brief 1x1の白色テクスチャのリソースハンドルを取得する関数
	bool GetWhite1x1TextureHandle(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle);
	/// @brief 1x1の黒色キューブマップテクスチャのリソースハンドルを取得する関数
	bool GetBlackCubeMapTextureHandle(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle);

	/// @brief リソースの配列サイズを取得する関数
	bool GetResourceArraySize(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle resourceHandle,
		size_t& outResourceArraySize);
	/// @brief レンダーターゲットのリソースハンドルを取得する関数
	bool GetRenderResourceHandle(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::RenderTargetHandle renderTargetHandle,
		QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle);

	/// @brief 深度ステンシルバッファのリソースハンドルを取得する関数
	bool GetDepthStencilResourceHandle(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& outDepthStencilHandle);

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

	/// @brief UAVバッファを作成する関数
	bool CreateUAVBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& outUAVBufferHandle,
		uint32_t width, uint32_t height, const std::wstring& name);

	/// @brief Object3d.GBuffer.PS用のルートリソースを作成する関数
	bool CreateObject3dGBufferRootResources(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle* rootResources,
		size_t rootResourcesSize);

	/// @brief リソースの状態を変更する関数
	bool TransitionResourceToState(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& resourceHandle,
		D3D12_RESOURCE_STATES newState);

	/// @brief カメラ位置バッファを作成する関数
	bool CreateCameraPosBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::wstring& name,
		QFE::GRAPHIC::DirectXResourceHandle& outCameraPosBufferHandle);

	/// @brief Object3dのワールド行列とワールドビュー射影行列を更新する関数
	void UpdateObject3dWVPMatrix(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& transformMatrixBufferHandle,
		const QFE::MATH::EulerTransform& transform, const QFE::MATH::Matrix4x4& viewProjectionMatrix);
	/// @brief 頂点のデータから頂点位置を取得する関数
	std::vector<QFE::MATH::Vector3> GetModelVertexPositions(
		const VertexData* vertices, size_t vertexCount);
	/// @brief 頂点バッファを作成する関数
	bool CreateVertexBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<VertexData>& vertexPositions,
		const std::string& meshName,
		QFE::GRAPHIC::DirectXResourceHandle& outVertexBufferHandle);
	/// @brief インデックスバッファを作成する関数
	bool CreateIndexBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<uint32_t>& indices,
		const std::string& meshName,
		QFE::GRAPHIC::DirectXResourceHandle& outIndexBufferHandle);

	/// @brief BLASを作成する関数
	bool CreateBLAS(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<VertexData>& vertices,
		const std::string& name,
		QFE::GRAPHIC::BLASHandle& outBLASHandle);
	/// @brief BLASのインスタンスバッファを更新する関数
	bool UpdateBLASInstanceBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<std::pair<QFE::GRAPHIC::BLASHandle, QFE::MATH::Matrix4x4>>& instances);

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

	bool TestRayTracingPSO(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
		QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
		const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& uvBufferHandle,
		const QFE::GRAPHIC::DirectXResourceHandle& instanceMetaBufferHandle,
		const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources);
}