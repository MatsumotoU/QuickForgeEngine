#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>

#include "math/vector/Vector3.h"

namespace QFE::GRAPHIC {
	/// @brief レイトレーシング用のボトムレベル加速構造（BLAS）を管理するクラス
	class BLAS {
	public:
		/// @brief BLASを構築します
		bool Create(
			ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4,
			const std::vector<QFE::MATH::Vector3>& vertices,
			const std::vector<uint32_t>& indices);
		/// @brief BLASの結果バッファを取得します
		ID3D12Resource* GetBLASResultBuffer() const { return blasResultBuffer_.Get(); }

	private:
		/// @brief BLASを構築するための頂点位置バッファを作成します
		bool CreateVertexPositionBuffer(ID3D12Device5* device5, const std::vector<QFE::MATH::Vector3>& vertices);
		/// @brief BLASを構築するためのインデックスバッファを作成します
		bool CreateIndexBuffer(ID3D12Device5* device5, const std::vector<uint32_t>& indices);
		/// @brief BLASを構築するためのリソースを作成します
		bool CreateBLASResource(
			ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4);

		bool isCreated_ = false; // BLASが作成済みかどうか

		// 頂点位置バッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexPositionBuffer_; 
		UINT vertexBufferSize_; // 頂点バッファのサイズ
		UINT vertexCount_; // 頂点の数
		// インデックスバッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
		UINT indexBufferSize_;
		UINT indexCount_;

		// BLAS（ボトムレベル加速構造）の実体バッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> blasResultBuffer_;
		// 構築の際の一時作業スペース（スクラッチバッファ）
		Microsoft::WRL::ComPtr<ID3D12Resource> blasScratchBuffer_;
	};
}
