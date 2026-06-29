#pragma once
#include <d3d12.h>
#include <wrl.h>

namespace QFE::GRAPHIC {

	class RaytracingAccelerationStructure final {
	public:
		RaytracingAccelerationStructure() = default;
		~RaytracingAccelerationStructure() = default;



		/// @brief テスト用のシンプルな三角形からBLASを構築します
		bool CreateTestBLAS(ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4);

		bool CreateTestTLAS(ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4);

		bool CreateShaderTables(ID3D12Device5* device5);

		ID3D12Resource* GetBLASResultBuffer() const { return blasResultBuffer_.Get(); }
		ID3D12Resource* GetTLASResultBuffer() const { return tlasResultBuffer_.Get(); }

	private:

		// BLAS（ボトムレベル加速構造）の実体バッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> blasResultBuffer_;
		// 構築の際の一時作業スペース（スクラッチバッファ）
		Microsoft::WRL::ComPtr<ID3D12Resource> blasScratchBuffer_;


		// TLAS用の命綱（メンバ変数）
		Microsoft::WRL::ComPtr<ID3D12Resource> tlasResultBuffer_;
		Microsoft::WRL::ComPtr<ID3D12Resource> tlasScratchBuffer_;
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceDescBuffer_; // インスタンス情報を載せるバッファ

		Microsoft::WRL::ComPtr<ID3D12Resource> testVertexBuffer;
	};

}