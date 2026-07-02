#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>

#include "BLAS.h"
#include "TLAS.h"

#include <unordered_map>
#include "memory/UniqueContainer.h"
#include "memory/SparseSets.h"

#include "dx12/GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {
	/// @brief レイトレーシング用の加速構造を管理するクラス
	class RaytracingAccelerationStructure final {
	public:
		RaytracingAccelerationStructure() = default;
		~RaytracingAccelerationStructure() = default;

		/// @brief レイトレーシング用の加速構造を初期化します
		void Initialize(ID3D12Device5* device5);
		/// @brief TLASに情報を登録します
		void UpdateTLAS(ID3D12GraphicsCommandList4* commandList4);

		/// @brief BLASを構築するためのリソースを作成します
		BLASHandle CreateBLAS(
			ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4,
			const std::vector<QFE::MATH::Vector3>& vertices, const std::string& name);

		/// @brief BLASのインスタンスを生成します
		BLASInstanceHandle CreateBLASInstance(BLASHandle handle, const QFE::MATH::Matrix4x4& transform);
		/// @brief BLASのあるインスタンスを削除します
		void RemoveBLASInstance(BLASInstanceHandle instanceHandle);
		/// @brief BLASのあるインスタンスの変換行列を更新します
		void UpdateBLASInstanceTransform(BLASInstanceHandle instanceHandle, const QFE::MATH::Matrix4x4& transform);

		/// @brief レイトレーシング用のシェーダーテーブルを作成します
		bool CreateShaderTables(ID3D12Device5* device5);

		/// @brief BLASの結果バッファを取得します
		ID3D12Resource* GetBLASResultBuffer(BLASHandle handle) const;
		/// @brief TLASの結果バッファを取得します
		ID3D12Resource* GetTLASResultBuffer() const;

	private:
		UniqueContainer<std::unique_ptr<BLAS>> blasContainer_; // BLASの管理用コンテナ
		SparseSet< RaytracingInstance> instanceSet_; // BLASインスタンスの管理用セット
		std::unique_ptr<TLAS> tlas_; // TLASの管理用インスタンス

		

		Microsoft::WRL::ComPtr<ID3D12Resource> testVertexBuffer;
	};

}