#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <functional>

#include <unordered_map>

#include "memory/SparseSets.h"
#include "DirectXResource.h"
#include "descriptors/Data/DescriptorHandles.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief オフスクリーンバッファの生成に必要な情報と関数をまとめた構造体
	struct OffscreenBufferInitializeInfo {
		ID3D12Device* device;
		uint32_t width;
		uint32_t height;
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_RENDER_TARGET_VIEW_DESC*)> assignRtvFunc;// Rtvを割り当てる関数
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_SHADER_RESOURCE_VIEW_DESC*)> assignSrvFunc;// Srvを割り当てる関数
	};

	/// @brief オフスクリーンを管理するクラス
	class OffscreenBuffer final {
	public:
		/// @brief オフスクリーンバッファを作成する
		uint32_t Create(OffscreenBufferInitializeInfo info);
		/// @brief 指定のリソースをクリアします
		void Clear(ID3D12GraphicsCommandList* commandList, uint32_t handle);

		/// @brief 指定のリソースを描画用にします
		bool SetRenderTarget(ID3D12GraphicsCommandList* commandList, uint32_t handle);
		/// @brief 指定のリソースを描画用にします
		bool SetTexture(ID3D12GraphicsCommandList* commandList, uint32_t handle);

		/// @brief 指定のリソースのRtvのハンドルを取得します
		DescriptorHandles GetRtvHandle(uint32_t handle) const;
		/// @brief 指定のリソースのRtvのハンドルのポインタを取得します
		const D3D12_CPU_DESCRIPTOR_HANDLE* GetRtvHandlePtr(uint32_t handle) const;
		/// @brief 指定のリソースのSrvのハンドルを取得します
		DescriptorHandles GetSrvHandle(uint32_t handle) const;

	private:
		std::unordered_map<uint32_t, DescriptorHandles> rtvHandles_;// オフスクリーンバッファのRtvのハンドルを管理するマップ
		std::unordered_map<uint32_t, DescriptorHandles> srvHandles_;// オフスクリーンバッファのSrvのハンドルを管理するマップ
		QFE::SparseSet<DirectXResource> offscreens_;// オフスクリーンバッファのリソースを管理するSparseSet
	};
}