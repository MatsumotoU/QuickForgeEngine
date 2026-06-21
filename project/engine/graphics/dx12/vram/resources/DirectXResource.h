#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <unordered_map>
#include "dx12/vram/descriptors/ViewTypeFlags.h"
#include "dx12/vram/descriptors/DescriptorHandles.h"

namespace QFE::GRAPHIC {
	/// @brief DirectX12のリソースをラップしたクラス
	class DirectXResource final {
	public:
		/// @brief リソースをVram上に作成する
		bool CreateResource(
			ID3D12Device* device, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_RESOURCE_STATES initialState,
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT, const D3D12_CLEAR_VALUE* clearValue = nullptr);
		/// @brief 外部で作成されたリソースをこのクラスに関連付ける
		bool SetExternalResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialState);

		/// @brief リソースをCPU側にマップする
		bool MapResource(UINT subresource = 0, const D3D12_RANGE* readRange = nullptr);
		/// @brief リソースのマップの全範囲を解除する
		bool UnmapResource();

		/// @brief リソースの状態を変更する
		bool TransitionResource(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);
		/// @brief 一つ前のリソースの状態に戻す
		bool TransitionResourceToBeforeState(ID3D12GraphicsCommandList* commandList);

		/// @brief リソースのマップされたCPU側のポインタを取得する
		template<typename T>
		T* GetMappedData() const {
			return static_cast<T*>(mappedData_);
		}
		/// @brief リソースのGPU側のアドレスを取得する
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const;
		/// @brief リソースそのものを取得する
		ID3D12Resource* GetResource() const;

		/// @brief あるビュータイプのデスクリプタハンドルをリソースに関連付けて保存します
		bool AddDescriptorHandle(ViewTypeFlags viewType, const DescriptorHandles& handles);
		/// @brief あるビュータイプのデスクリプタハンドルを取得します
		const DescriptorHandles* GetDescriptorHandle(ViewTypeFlags viewType) const;
		/// @brief リソースに関連付けられたビューのタイプを管理するフラグを取得します
		ViewTypeFlags GetViewTypes() const;
		/// @brief あるビュータイプがリソースに関連付けられているかを確認します
		bool HasTypeOfView(ViewTypeFlags viewType) const;

		/// @brief マップされたリソースの1要素あたりのサイズ（バイト単位）を取得します。これは主にバッファリソースで使用されます。
		size_t GetStrideInBytes() const;
		/// @brief マップされたリソースの1要素あたりのサイズ（バイト単位）を設定します。これは主にバッファリソースで使用されます。
		void SetStrideInBytes(size_t strideInBytes);

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_;// VRAM上のリソース
		void* mappedData_;// マップされたリソースのCPU側のポインタ
		size_t strideInBytes_;// マップされたリソースの1要素あたりのサイズ（バイト単位）
		bool isSetStrideInBytes_;// strideInBytes_が有効かどうかを示すフラグ

		D3D12_RESOURCE_STATES currentState_;// リソースの現在の状態
		D3D12_RESOURCE_STATES beforeState_;// リソースの前の状態

		D3D12_RESOURCE_DESC resourceDesc_;// リソースの説明

		ViewTypeFlags viewTypes_;// リソースに関連付けられたビューのタイプを管理するフラグ
		std::unordered_map<ViewTypeFlags, DescriptorHandles> descriptorHandles_;// リソースに関連付けられたデスクリプタのハンドルを管理するマップ
	};
}