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
struct InstanceMetaCPU;	

namespace QFE::GRAPHIC {
	class D3D12GraphicEngine;
}

/// @file DirectXResourceFramework.h
/// @brief DirectXリソースに関するフレームワーク関数を提供するヘッダーファイル
namespace QFE::FRAMEWORK {

	/// @brief ファイルからテクスチャを読み込む関数
	bool LoadTextureFromFile(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::string& filePath,
		QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle);
	/// @brief 1x1の白色テクスチャのリソースハンドルを取得する関数
	bool GetWhite1x1TextureHandle(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle);
	/// @brief 1x1の黒色キューブマップテクスチャのリソースハンドルを取得する関数
	bool GetBlackCubeMapTextureHandle(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle);

	/// @brief UAVバリアの状態遷移を行う関数
	bool UAVBarrierTransition(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& resourceHandle);

	/// @brief リソースの状態を変更する関数
	bool TransitionResourceToState(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& resourceHandle,
		D3D12_RESOURCE_STATES newState);
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

	/// @brief Object3d.GBuffer.PS用のルートリソースを作成する関数
	bool CreateObject3dGBufferRootResources(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle* rootResources,
		size_t rootResourcesSize);
	/// @brief カメラ位置バッファを作成する関数
	bool CreateCameraPosBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::wstring& name,
		QFE::GRAPHIC::DirectXResourceHandle& outCameraPosBufferHandle);

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
		const std::vector<uint32_t>& indices,
		const std::string& name,
		QFE::GRAPHIC::BLASHandle& outBLASHandle);
	/// @brief UAVバッファを作成する関数
	bool CreateUAVBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& outUAVBufferHandle,
		uint32_t width, uint32_t height, const std::wstring& name);

	/// @brief Object3dのワールド行列とワールドビュー射影行列を更新する関数
	void UpdateObject3dWVPMatrix(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& transformMatrixBufferHandle,
		const QFE::MATH::EulerTransform& transform, const QFE::MATH::Matrix4x4& viewProjectionMatrix);
	/// @brief BLASのインスタンスバッファを更新する関数
	bool UpdateBLASInstanceBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<std::pair<QFE::GRAPHIC::BLASHandle, QFE::MATH::Matrix4x4>>& instances);

	/// @brief 頂点のデータから頂点位置を取得する関数
	std::vector<QFE::MATH::Vector3> GetModelVertexPositions(
		const VertexData* vertices, size_t vertexCount);

	/// @brief バッファの容量を確保し、データをアップロードする関数
	// モデル群（modelDataMap）からグローバルバッファを平坦化する。
	// - globalUVs: [u0,v0, u1,v1, ...]
	// - globalTriIndices: flattened indices [i0,i1,i2, i3,i4,i5, ...] (各 tri は 3 要素)
	// - outInstanceMeta: メッシュ（またはメッシュ単位のエントリ）ごとのメタ情報（InstanceID と一致させること）
	bool EnsureBufferCapacityAndUpload(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle& inOutHandle,
		const void* data, size_t byteSize, UINT elementStride,
		const std::string& name);

	/// @brief グローバルメッシュバッファをアップロードする関数
	// GPU に平坦化済データをアップロードして StructuredBuffer (SRV) を作る。
	// 成功時に outXXXHandle にリソースハンドルを格納する。
	// 注意: CreateResourceView の srvDesc のフィールド名はプロジェクト実装に合わせて調整してください。
	bool UploadGlobalMeshBuffers(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<float>& globalUVs,
		const std::vector<uint32_t>& globalTriIndices,
		const std::vector<InstanceMetaCPU>& instanceMeta,
		QFE::GRAPHIC::DirectXResourceHandle& outUVHandle,
		QFE::GRAPHIC::DirectXResourceHandle& outTriHandle,
		QFE::GRAPHIC::DirectXResourceHandle& outInstanceMetaHandle);
}