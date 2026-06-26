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

	private:
		// BLAS（ボトムレベル加速構造）の実体バッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> blasResultBuffer_;
		// 構築の際の一時作業スペース（スクラッチバッファ）
		Microsoft::WRL::ComPtr<ID3D12Resource> blasScratchBuffer_;
	};

}