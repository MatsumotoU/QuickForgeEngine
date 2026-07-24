#include "DirectXResourceContainer.h"

#include <bit>
using namespace QFE::GRAPHIC;

namespace {
	ViewTypeFlags RootParameterTypeToViewType(D3D12_ROOT_PARAMETER_TYPE rootParamType) {
		switch (rootParamType) {
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
			return ViewTypeFlags::ConstantBufferView;
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: // SRV
			return ViewTypeFlags::ShaderResourceView;
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
			return ViewTypeFlags::UnorderedAccessView;
		default:
			return ViewTypeFlags::None;
		}
	}
}

void DirectXResourceContainer::Initialize(
	DirectXResourceContainerInitializeInfo initializeInfo) {

	info_ = initializeInfo;
}

void DirectXResourceContainer::EndFrame() {
	internalResources.clear();
}

DirectXResourceHandle DirectXResourceContainer::CreateResource(
	ID3D12Device* device,const D3D12_RESOURCE_DESC& resourceDesc,
	D3D12_RESOURCE_STATES initialState,D3D12_HEAP_TYPE heapType, const D3D12_CLEAR_VALUE* clearValue) {

	DirectXResource resource;
	if (!resource.CreateResource(device, resourceDesc, initialState, heapType, clearValue)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create resource in DirectXResourceContainer::CreateResource", SystemError::Abort);
		return DirectXResourceHandle::Invalid;
	}

	return static_cast<DirectXResourceHandle>(resources.push_back(std::move(resource)));
}

DirectXResourceHandle QFE::GRAPHIC::DirectXResourceContainer::CreateBuffer(ID3D12Device* device, size_t bufferSize) {
	// * Resourceを生成する * //
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC bufferDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = static_cast<UINT64>(bufferSize);
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	return CreateResource(device, bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
}

DirectXResourceHandle QFE::GRAPHIC::DirectXResourceContainer::RegisterExternalResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialState) {
	DirectXResource tempResource;
	if(tempResource.SetExternalResource(resource, initialState)) {
		return static_cast<DirectXResourceHandle>(resources.push_back(std::move(tempResource)));
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to register external resource in DirectXResourceContainer::RegisterExternalResource", SystemError::Abort);
		return DirectXResourceHandle::Invalid;
	}
	return DirectXResourceHandle::Invalid;
}

bool QFE::GRAPHIC::DirectXResourceContainer::SetResourceName(DirectXResourceHandle handle, const std::wstring& name) {
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::SetResourceName", SystemError::Abort);
		return false;
	}
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		return resource.SetName(name);
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::SetResourceName", SystemError::Abort);
		return false;
	}
}

void DirectXResourceContainer::MapResource(DirectXResourceHandle handle, UINT subresource, const D3D12_RANGE* readRange) {
	// 引数の検査
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::MapResource", SystemError::Abort);
		return;
	}
	// リソースが存在するかの確認
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		// リソースをマップする
		if (!resource.MapResource(subresource, readRange)) {
			QFE_REPORT_SYSTEM_ERROR("Failed to map resource in DirectXResourceContainer::MapResource", SystemError::Abort);
			return;
		}
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::MapResource", SystemError::Abort);
		return;
	}
}

void DirectXResourceContainer::CreateResourceView(DirectXResourceHandle handle, CreateViewInfo createViewInfo) {
	// 引数の検査
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::CreateResourceView", SystemError::Abort);
		return;
	}
	// リソースが存在するかの確認
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		ID3D12Resource* d3dResource = resource.GetResource();
		if (!d3dResource) {
			QFE_REPORT_SYSTEM_ERROR("Failed to get D3D resource in DirectXResourceContainer::CreateResourceView", SystemError::Abort);
			return;
		}

		// ビュータイプに応じてデスクリプタハンドルを割り当て、リソースに関連付けて保存
		DescriptorHandles handles;
		ViewTypeFlags viewType = createViewInfo.viewType;
		// RTV
		if ((viewType & ViewTypeFlags::RenderTargetView) != ViewTypeFlags::None) {
			handles = info_.assignRtvFunc(d3dResource, &createViewInfo.rtvDesc);
			resource.AddDescriptorHandle(ViewTypeFlags::RenderTargetView, handles);
			QFE_LOG("RTV view created successfully in DirectXResourceContainer::CreateResourceView");
		}
		// SRV
		if ((viewType & ViewTypeFlags::ShaderResourceView) != ViewTypeFlags::None) {
			handles = info_.assignSrvFunc(d3dResource, &createViewInfo.srvDesc);
			resource.AddDescriptorHandle(ViewTypeFlags::ShaderResourceView, handles);
			QFE_LOG("SRV view created successfully in DirectXResourceContainer::CreateResourceView");
		}
		// DSV
		if ((viewType & ViewTypeFlags::DepthStencilView) != ViewTypeFlags::None) {
			handles = info_.assignDsvFunc(d3dResource, &createViewInfo.dsvDesc);
			resource.AddDescriptorHandle(ViewTypeFlags::DepthStencilView, handles);
			QFE_LOG("DSV view created successfully in DirectXResourceContainer::CreateResourceView");
		}
		// UAV
		if ((viewType & ViewTypeFlags::UnorderedAccessView) != ViewTypeFlags::None) {
			handles = info_.assignUavFunc(d3dResource, &createViewInfo.uavDesc);
			resource.AddDescriptorHandle(ViewTypeFlags::UnorderedAccessView, handles);
			QFE_LOG("UAV view created successfully in DirectXResourceContainer::CreateResourceView");
		}
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::CreateResourceView", SystemError::Abort);
		return;
	}
	QFE_LOG("Resource view created successfully in DirectXResourceContainer::CreateResourceView");
}

void QFE::GRAPHIC::DirectXResourceContainer::CreateTextureResourceView(
	DirectXResourceHandle handle, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc) {

	// リソースが存在するかの確認
	DescriptorHandles handles;
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		ID3D12Resource* d3dResource = resource.GetResource();
		if (!d3dResource) {
			QFE_REPORT_SYSTEM_ERROR("Failed to get D3D resource in DirectXResourceContainer::CreateTextureResourceView", SystemError::Abort);
			return;
		}

		handles = info_.assignTextureFunc(d3dResource, &srvDesc);
		resource.AddDescriptorHandle(ViewTypeFlags::ShaderResourceView, handles);
		QFE_LOG("SRV view created successfully in DirectXResourceContainer::CreateTextureResourceView");
	}
	return;
}

void QFE::GRAPHIC::DirectXResourceContainer::UploadResource(
	DirectXResourceHandle handle, std::vector<D3D12_SUBRESOURCE_DATA> subresources,
	ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {

	// 指定のリソースがあるか
	DirectXResource* destinationResource = GetDirectXResource(handle);
	// リソースのバリアをコピー可能な状態に変更
	destinationResource->TransitionResource(commandList, D3D12_RESOURCE_STATE_COPY_DEST);

	// コピーに必要な中間リソースのサイズを取得
	UINT64 uploadBufferSize = GetRequiredIntermediateSize(destinationResource->GetResource(), 0, static_cast<UINT>(subresources.size()));

	// 中間リソースを生成します
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC bufferDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = uploadBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(intermediateResource.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));

	// アップロード用のリソースにサブリソースデータを書き込む
	UpdateSubresources(commandList, GetResource(handle), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// アップロードに使ったリソースを保存して、EndFrameで解放できるようにする
	internalResources.push_back(std::move(intermediateResource));

	// リソースのバリアをコピー前の状態に変更
	destinationResource->TransitionResourceToBeforeState(commandList);

	QFE_LOG("Resource uploaded successfully in DirectXResourceContainer::UploadResource");
}

bool DirectXResourceContainer::HasResourceType(DirectXResourceHandle handle, ViewTypeFlags viewType) const {
	if(handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::HasResourceType", SystemError::Abort);
		return false;
	}
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		const DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		return resource.HasTypeOfView(viewType);
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::HasResourceType", SystemError::Abort);
	}

	return false;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXResourceContainer::GetDescriptorHandleCPU(DirectXResourceHandle handle, ViewTypeFlags viewType) const {
	// タイプが複数指定されている場合はエラー
	uint32_t value = static_cast<uint32_t>(viewType);
	if (value == 0 || std::has_single_bit(value) == false) {
		QFE_REPORT_SYSTEM_ERROR("Invalid view type in DirectXResourceContainer::GetDescriptorHandle", SystemError::Abort);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
	// ハンドルが無効な場合はエラー
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::GetDescriptorHandle", SystemError::Abort);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
	// リソースが存在するかの確認
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		const DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		const DescriptorHandles* handles = resource.GetDescriptorHandle(viewType);
		if (handles) {
			return handles->cpuHandle_;
		} else {
			QFE_REPORT_SYSTEM_ERROR("Descriptor handle for the specified view type not found in DirectXResourceContainer::GetDescriptorHandle", SystemError::Abort);
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::GetDescriptorHandle", SystemError::Abort);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE QFE::GRAPHIC::DirectXResourceContainer::GetDescriptorHandleGPU(DirectXResourceHandle handle, ViewTypeFlags viewType) const {
	// タイプが複数指定されている場合はエラー
	uint32_t value = static_cast<uint32_t>(viewType);
	if (value == 0 || std::has_single_bit(value) == false) {
		QFE_REPORT_SYSTEM_ERROR("Invalid view type in DirectXResourceContainer::GetDescriptorHandleGPU", SystemError::Abort);
		return D3D12_GPU_DESCRIPTOR_HANDLE();
	}
	// ハンドルが無効な場合はエラー
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::GetDescriptorHandleGPU", SystemError::Abort);
		return D3D12_GPU_DESCRIPTOR_HANDLE();
	}
	// リソースが存在するかの確認
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		const DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		const DescriptorHandles* handles = resource.GetDescriptorHandle(viewType);
		if (handles) {
			return handles->gpuHandle_;
		} else {
			QFE_REPORT_SYSTEM_ERROR("Descriptor handle for the specified view type not found in DirectXResourceContainer::GetDescriptorHandleGPU", SystemError::Abort);
			return D3D12_GPU_DESCRIPTOR_HANDLE();
		}
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::GetDescriptorHandleGPU", SystemError::Abort);
		return D3D12_GPU_DESCRIPTOR_HANDLE();
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE* DirectXResourceContainer::GetDescriptorHandleCpuPtr(DirectXResourceHandle handle, ViewTypeFlags viewType) const {
	// タイプが複数指定されている場合はエラー
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::GetDescriptorHandleCpuPtr", SystemError::Abort);
		return nullptr;
	}
	// リソースが存在するかの確認
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		const DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		const DescriptorHandles* handles = resource.GetDescriptorHandle(viewType);
		if (handles) {
			return &(handles->cpuHandle_);
		} else {
			QFE_REPORT_SYSTEM_ERROR("Descriptor handle for the specified view type not found in DirectXResourceContainer::GetDescriptorHandleCpuPtr", SystemError::Abort);
			return nullptr;
		}
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::GetDescriptorHandleCpuPtr", SystemError::Abort);
		return nullptr;
	}
}

const D3D12_GPU_DESCRIPTOR_HANDLE* DirectXResourceContainer::GetDescriptorHandleGpuPtr(DirectXResourceHandle handle, ViewTypeFlags viewType) const {
	// タイプが複数指定されている場合はエラー
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::GetDescriptorHandleGpuPtr", SystemError::Abort);
		return nullptr;
	}
	// リソースが存在するかの確認
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		const DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		const DescriptorHandles* handles = resource.GetDescriptorHandle(viewType);
		if (handles) {
			return &(handles->gpuHandle_);
		} else {
			QFE_REPORT_SYSTEM_ERROR("Descriptor handle for the specified view type not found in DirectXResourceContainer::GetDescriptorHandleGpuPtr", SystemError::Abort);
			return nullptr;
		}
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::GetDescriptorHandleGpuPtr", SystemError::Abort);
		return nullptr;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXResourceContainer::GetDescriptorHandleCPU(
	DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const {
	return GetDescriptorHandleCPU(handle, RootParameterTypeToViewType(parameterType));
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXResourceContainer::GetDescriptorHandleGPU(
	DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const {
	return GetDescriptorHandleGPU(handle, RootParameterTypeToViewType(parameterType));
}

const D3D12_CPU_DESCRIPTOR_HANDLE* DirectXResourceContainer::GetDescriptorHandleCpuPtr(
	DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const {
	return GetDescriptorHandleCpuPtr(handle, RootParameterTypeToViewType(parameterType));
}

const D3D12_GPU_DESCRIPTOR_HANDLE* DirectXResourceContainer::GetDescriptorHandleGpuPtr(
	DirectXResourceHandle handle, D3D12_ROOT_PARAMETER_TYPE parameterType) const {
	return GetDescriptorHandleGpuPtr(handle, RootParameterTypeToViewType(parameterType));
}

D3D12_GPU_VIRTUAL_ADDRESS DirectXResourceContainer::GetGpuVirtualAddress(DirectXResourceHandle handle) const {	
	return GetResource(handle)->GetGPUVirtualAddress();
}

D3D12_VERTEX_BUFFER_VIEW DirectXResourceContainer::GetVertexBufferView(DirectXResourceHandle handle) const {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = GetGpuVirtualAddress(handle);
	ID3D12Resource* resource = GetResource(handle);
	if (resource) {
		D3D12_RESOURCE_DESC desc = resource->GetDesc();
		vertexBufferView.SizeInBytes = static_cast<UINT>(desc.Width);
		vertexBufferView.StrideInBytes = static_cast<UINT>(GetResourceStrideInBytes(handle));
		return vertexBufferView;
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to get resource in DirectXResourceContainer::GetVertexBufferView", SystemError::Abort);
	}
	return vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW DirectXResourceContainer::GetIndexBufferView(DirectXResourceHandle handle) const {
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	indexBufferView.BufferLocation = GetGpuVirtualAddress(handle);
	ID3D12Resource* resource = GetResource(handle);
	if (resource) {
		D3D12_RESOURCE_DESC desc = resource->GetDesc();
		indexBufferView.SizeInBytes = static_cast<UINT>(desc.Width);
		indexBufferView.Format = DXGI_FORMAT_R32_UINT; // インデックスバッファのフォーマットは32ビット整数と仮定
		return indexBufferView;
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to get resource in DirectXResourceContainer::GetIndexBufferView", SystemError::Abort);
	}
	return indexBufferView;
}

ID3D12Resource* DirectXResourceContainer::GetResource(DirectXResourceHandle handle) const {
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::GetResource", SystemError::Abort);
		return nullptr;
	}

	if (resources.Contains(static_cast<uint32_t>(handle))) {
		return resources.at(static_cast<uint32_t>(handle)).GetResource();
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::GetResource", SystemError::Abort);
		return nullptr;
	}
}

ViewTypeFlags DirectXResourceContainer::GetResourceViewType(DirectXResourceHandle handle) const {
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::GetResourceViewType", SystemError::Abort);
		return ViewTypeFlags::None;
	}
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		return resources.at(static_cast<uint32_t>(handle)).GetViewTypes();
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::GetResourceViewType", SystemError::Abort);
		return ViewTypeFlags::None;
	}
}

size_t QFE::GRAPHIC::DirectXResourceContainer::GetResourceStrideInBytes(DirectXResourceHandle handle) const {
	return resources.at(static_cast<uint32_t>(handle)).GetStrideInBytes();
}

void QFE::GRAPHIC::DirectXResourceContainer::SetResourceStrideInBytes(DirectXResourceHandle handle, size_t strideInBytes) {
	resources.at(static_cast<uint32_t>(handle)).SetStrideInBytes(strideInBytes);
}

size_t QFE::GRAPHIC::DirectXResourceContainer::GetResourceSizeInBytes(DirectXResourceHandle handle) const {
	return resources.at(static_cast<uint32_t>(handle)).GetResourceSizeInBytes();
}

UINT QFE::GRAPHIC::DirectXResourceContainer::GetResourceHeight(DirectXResourceHandle handle) const {
	return resources.at(static_cast<uint32_t>(handle)).GetResourceHeight();
}

UINT QFE::GRAPHIC::DirectXResourceContainer::GetResourceWidth(DirectXResourceHandle handle) const {
	return resources.at(static_cast<uint32_t>(handle)).GetResourceWidth();
}

bool DirectXResourceContainer::TransitionResource(
	DirectXResourceHandle handle, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState) {
	// 引数の検査
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::TransitionResource", SystemError::Abort);
		return false;
	}
	// リソースが存在するかの確認
	if (resources.Contains(static_cast<uint32_t>(handle))) {
		DirectXResource& resource = resources.at(static_cast<uint32_t>(handle));
		if (!resource.TransitionResource(commandList, newState)) {
			QFE_LOG("Failed to transition resource in DirectXResourceContainer::TransitionResource");
			return false;
		}
		return true;
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::TransitionResource", SystemError::Abort);
	}
	return false;
}

DirectXResource* DirectXResourceContainer::GetDirectXResource(DirectXResourceHandle handle) {
	if (handle == DirectXResourceHandle::Invalid) {
		QFE_REPORT_SYSTEM_ERROR("Invalid resource handle in DirectXResourceContainer::GetDirectXResource", SystemError::Abort);
		return nullptr;
	}
	if(resources.Contains(static_cast<uint32_t>(handle))) {
		return &resources.at(static_cast<uint32_t>(handle));
	} else {
		QFE_REPORT_SYSTEM_ERROR("Resource handle not found in DirectXResourceContainer::GetDirectXResource", SystemError::Abort);
	}
	return nullptr;
}
