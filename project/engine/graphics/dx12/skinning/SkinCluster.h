#pragma once
#include "math/MathInclude.h"
#include <vector>
#include <span>
#include <utility>
#include <d3d12.h>
#include <wrl.h>
#include "../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

namespace QFE::GRAPHIC {
	struct SkinCluster {
		std::vector<QFE::MATH::Matrix4x4> inverseBindPoseMatrices; // ジョイントの逆バインドポーズ行列の配列
		Microsoft::WRL::ComPtr<ID3D12Resource> inverseBindPoseBuffer; // ジョイントの逆バインドポーズ行列を格納するGPUバッファ
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView; // ジョイントの影響情報を格納するGPUバッファのビュー
		std::span<VertexInfluence> mappedInfluenceData; // ジョイントの影響情報を格納するCPU側のメモリのスパン
		Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource; // ジョイントのパレット行列を格納するGPUバッファ
		std::span<QFE::MATH::Matrix4x4> mappedPaletteData;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteDescriptorHandle; // ジョイントのパレット行列を格納するGPUバッファのビュー
	};
}
