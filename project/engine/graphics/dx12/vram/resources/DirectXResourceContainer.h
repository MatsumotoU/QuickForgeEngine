#pragma once
#include "DirectXResource.h"
#include "memory/SparseSets.h"

#include "graphics/dx12/vram/descriptors/ViewTypeFlags.h"
#include <functional>
// UploadResourceの引数でD3D12_RESOURCE_DESCを使うために必要
#include "DirectXTex/d3dx12.h"

#include "graphics/dx12/GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {

	/// @brief DirectX12のリソースをまとめて管理するクラスの初期化に必要な情報と関数をまとめた構造体
	struct DirectXResourceContainerInitializeInfo {
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_RENDER_TARGET_VIEW_DESC*)> assignRtvFunc;// Rtvを割り当てる関数
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_SHADER_RESOURCE_VIEW_DESC*)> assignSrvFunc;// Srvを割り当てる関数
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_DEPTH_STENCIL_VIEW_DESC*)> assignDsvFunc;// Dsvを割り当てる関数
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_UNORDERED_ACCESS_VIEW_DESC*)> assignUavFunc;// Uavを割り当てる関数
	};

	/// @brief ビューのタイプと説明をまとめた構造体
	struct CreateViewInfo {
		ViewTypeFlags viewType;// 作成するビューのタイプ
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;// 作るRTVの説明
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;// 作るSRVの説明
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;// 作るDSVの説明
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;// 作るUAVの説明
	};

	/// @brief DirectX12のリソースをまとめて管理するクラス
	class DirectXResourceContainer final {
	public:
		/// @brief DirectX12のリソースをまとめて管理するクラスを初期化します。
		void Initialize(DirectXResourceContainerInitializeInfo initializeInfo);
		/// @brief アップロードするのに使ったリソースを全て解放します。
		void EndFrame();

		/// @brief 定数バッファを作成し、コンテナに追加する
		DirectXResourceHandle CreateResource(
			ID3D12Device* device,
			const D3D12_RESOURCE_DESC& resourceDesc,
			D3D12_RESOURCE_STATES initialState,
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
			const D3D12_CLEAR_VALUE* clearValue = nullptr);
		/// @brief 定数バッファを作成し、コンテナに追加する
		DirectXResourceHandle CreateBuffer(ID3D12Device* device, size_t bufferSize);
		/// @brief 外部のリソースをコンテナに追加する
		DirectXResourceHandle RegisterExternalResource(
			Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialState);

		/// @brief あるリソースハンドルに対応するリソースの名前を設定する
		bool SetResourceName(DirectXResourceHandle handle, const std::wstring& name);

		/// @brief あるリソースハンドルに対応するリソースの状態を変更する
		bool TransitionResource(DirectXResourceHandle handle, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);
		/// @brief あるハンドルのリソースをマッピングする
		void MapResource(DirectXResourceHandle handle, UINT subresource = 0, const D3D12_RANGE* readRange = nullptr);
		
		/// @brief リソースのビューを生成します
		void CreateResourceView(DirectXResourceHandle handle, CreateViewInfo createViewInfo);
		/// @brief リソースをGPUにアップロードします
		void UploadResource(
			DirectXResourceHandle handle, std::vector<D3D12_SUBRESOURCE_DATA> subresources,
			ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		/// @brief あるハンドルがあるビュータイプのリソースを持っているか
		bool HasResourceType(DirectXResourceHandle handle, ViewTypeFlags viewType) const;
		
		/// @brief あるハンドルの対応するビューのCPUディスクリプタハンドルを取得します
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandleCPU(DirectXResourceHandle handle, ViewTypeFlags viewType) const;
		/// @brief あるハンドルの対応するビューのGPUディスクリプタハンドルを取得します
		D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandleGPU(DirectXResourceHandle handle, ViewTypeFlags viewType) const;
		/// @brief あるハンドルの対応するビューのCPUディスクリプタハンドルへのポインタを取得します
		const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptorHandleCpuPtr(DirectXResourceHandle handle, ViewTypeFlags viewType) const;
		/// @brief あるハンドルの対応するビューのGPUディスクリプタハンドルへのポインタを取得します
		const D3D12_GPU_DESCRIPTOR_HANDLE* GetDescriptorHandleGpuPtr(DirectXResourceHandle handle, ViewTypeFlags viewType) const;
		/// @brief あるハンドルの対応するビューのCPUディスクリプタハンドルを取得します
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandleCPU(DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const;
		/// @brief あるハンドルの対応するビューのGPUディスクリプタハンドルを取得します
		D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandleGPU(DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const;
		/// @brief あるハンドルの対応するビューのCPUディスクリプタハンドルへのポインタを取得します
		const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptorHandleCpuPtr(DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const;
		/// @brief あるハンドルの対応するビューのGPUディスクリプタハンドルへのポインタを取得します
		const D3D12_GPU_DESCRIPTOR_HANDLE* GetDescriptorHandleGpuPtr(DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const;

		/// @brief あるハンドルの対応するビューのGPU仮想アドレスを取得します
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(DirectXResourceHandle handle) const;
		/// @brief あるハンドルの対応するビューの頂点バッファビューを取得します
		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(DirectXResourceHandle handle) const;
		/// @brief あるハンドルの対応するビューのインデックスバッファビューを取得します
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(DirectXResourceHandle handle) const;

		/// @brief あるリソースハンドルに対応するリソースを取得する
		ID3D12Resource* GetResource(DirectXResourceHandle handle) const;
		/// @brief あるリソースハンドルに対応するビューのタイプを取得する
		ViewTypeFlags GetResourceViewType(DirectXResourceHandle handle) const;

		/// @brief あるリソースハンドルに対応するリソースの1要素あたりのサイズ（バイト単位）を取得します。これは主にバッファリソースで使用されます。
		size_t GetResourceStrideInBytes(DirectXResourceHandle handle) const;
		/// @brief あるリソースハンドルに対応するリソースの1要素あたりのサイズ（バイト単位）を設定します。これは主にバッファリソースで使用されます。
		void SetResourceStrideInBytes(DirectXResourceHandle handle, size_t strideInBytes);
		/// @brief あるリソースハンドルに対応するリソースのサイズ（バイト単位）を取得します。これは主にバッファリソースで使用されます。
		size_t GetResourceSizeInBytes(DirectXResourceHandle handle) const;

		/// @brief あるリソースハンドルに対応するリソースの幅を取得します。これは主にテクスチャリソースで使用されます。
		UINT GetResourceWidth(DirectXResourceHandle handle) const;
		/// @brief あるリソースハンドルに対応するリソースの高さを取得します。これは主にテクスチャリソースで使用されます。
		UINT GetResourceHeight(DirectXResourceHandle handle) const;

		/// @brief あるリソースハンドルに対応するリソースのマップされたCPU側のポインタを取得する
		template<typename T>
		T* GetMappedData(DirectXResourceHandle handle) {
			DirectXResource* resource = GetDirectXResource(handle);
			if (resource) {
				return resource->GetMappedData<T>();
			}
			return nullptr;
		}

	private:
		/// @brief あるリソースハンドルに対応するDirectXResourceを取得する
		DirectXResource* GetDirectXResource(DirectXResourceHandle handle);

		DirectXResourceContainerInitializeInfo info_;// リソースコンテナの初期化に必要な情報と関数をまとめた構造体
		SparseSet<DirectXResource> resources;// リソースのコンテナ
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> internalResources;// アップロードするのに使ったリソースを全て保存するベクター
	};
}