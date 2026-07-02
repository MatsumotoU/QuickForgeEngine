#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>

#include "BLAS.h"
#include "math/matrix/Matrix4x4.h"

#include <unordered_map>
#include "memory/UniqueContainer.h"

#include "dx12/GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {
	/// @brief レイトレーシング用の加速構造を管理するクラス
	class RaytracingAccelerationStructure final {
	public:
		RaytracingAccelerationStructure() = default;
		~RaytracingAccelerationStructure() = default;

		/// @brief BLASを構築するためのリソースを作成します
		BLASHandle CreateBLAS(
			ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4,
			const std::vector<QFE::MATH::Vector3>& vertices, const std::string& name);

		/// @brief レイトレーシング用のシェーダーテーブルを作成します
		bool CreateShaderTables(ID3D12Device5* device5);

		/// @brief BLASの結果バッファを取得します
		ID3D12Resource* GetBLASResultBuffer(BLASHandle handle) const;
		/// @brief TLASの結果バッファを取得します
		ID3D12Resource* GetTLASResultBuffer() const { return tlasResultBuffer_.Get(); }

	private:
		UniqueContainer<std::unique_ptr<BLAS>> blasContainer_; // BLASの管理用コンテナ

		

		Microsoft::WRL::ComPtr<ID3D12Resource> testVertexBuffer;
	};

}