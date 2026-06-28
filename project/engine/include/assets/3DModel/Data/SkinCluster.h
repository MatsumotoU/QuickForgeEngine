#pragma once
#include "engine/include/core/Math/Matrix/Matrix4x4.h"
#include "engine/include/core/Memory/SafeVector.h"

#include <d3d12.h>
#include <wrl.h>
#include <span>
#include <array>

namespace QFE
{
	const uint32_t MaxInfluencesPerVertex = 4; // 頂点ごとの最大影響数
	struct VertexInfluence
	{
		std::array<float, MaxInfluencesPerVertex> weights;
		std::array<int32_t, MaxInfluencesPerVertex> jointIndices;
	};
	struct WellForGPU
	{
		Matrix4x4 skeletonSpaceMatrix; // スケルトンスペース行列
		Matrix4x4 skeletonSpaceInverseTransposeMatrix; // スケルトンスペースの逆転置行列
	};
	struct SkinCluster {
		SafeVector<Matrix4x4> inverseBindPoseMatrices; // 逆バインドポーズ行列の配列
		Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource; // GPU上の影響行列リソース
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView; // 影響行列のバッファビュー
		std::span< VertexInfluence> mappedInfluences; // マップされた影響行列のスパン
		Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource; // GPU上のパレットリソース
		std::span<WellForGPU> mappedPalette;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
	};
}
