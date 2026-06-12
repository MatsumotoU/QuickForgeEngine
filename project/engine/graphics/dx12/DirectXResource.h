#pragma once
#include <wrl.h>
#include <d3d12.h>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief DirectX12のリソースをラップしたクラス
	class DirectXResource final {
	public:
		/// @brief リソースをVram上に作成する
		bool CreateResource(
			ID3D12Device* device, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_RESOURCE_STATES initialState,
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT, const D3D12_CLEAR_VALUE* clearValue = nullptr);

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
		ID3D12Resource* GetResource();

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_;// VRAM上のリソース
		void* mappedData_;// マップされたリソースのCPU側のポインタ

		D3D12_RESOURCE_STATES currentState_;// リソースの現在の状態
		D3D12_RESOURCE_STATES beforeState_;// リソースの前の状態

		D3D12_RESOURCE_DESC resourceDesc_;// リソースの説明
	};
}