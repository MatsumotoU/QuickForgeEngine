#pragma once
#include "DirectXResource.h"
#include "memory/SparseSets.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief DirectX12のリソースをまとめて管理するクラス
	class DirectXResourceContainer final {
		/// @brief 定数バッファを作成し、コンテナに追加する
		uint32_t CreateConstantBuffer(ID3D12Device* device, size_t bufferSize);

	public:
		SparseSet<DirectXResource> resources;// リソースのコンテナ
	};
}