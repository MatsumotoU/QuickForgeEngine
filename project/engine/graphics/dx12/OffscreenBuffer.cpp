#include "OffscreenBuffer.h"

using namespace QFE::GRAPHIC::INTERNAL;

namespace {
	const FLOAT kClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
}

uint32_t QFE::GRAPHIC::INTERNAL::OffscreenBuffer::Create(OffscreenBufferInitializeInfo info) {
	DirectXResource resource;
	// リソースの説明を設定
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = static_cast<UINT64>(info.width);
	desc.Height = static_cast<UINT>(info.height);
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	// リソースの初期状態をレンダーターゲットに設定
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
	// クリア値の設定（オフスクリーンバッファは通常、レンダーターゲットとして使用されるため、色のクリア値を指定）
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	clearValue.Color[0] = kClearColor[0];
	clearValue.Color[1] = kClearColor[1];
	clearValue.Color[2] = kClearColor[2];
	clearValue.Color[3] = kClearColor[3];
	// リソースの作成
	resource.CreateResource(info.device, desc, initialState, heapType, &clearValue);
	uint32_t handle = offscreens_.push_back(std::move(resource));

	// Rtvの割り当て
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = desc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvHandles_[handle] = info.assignRtvFunc(resource.GetResource(), &rtvDesc);
	// Srvの割り当て
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvHandles_[handle] = info.assignSrvFunc(resource.GetResource(), &srvDesc);

	// オフスクリーンバッファを管理するためのSparseSetに追加
	return handle;
}

void QFE::GRAPHIC::INTERNAL::OffscreenBuffer::Clear(ID3D12GraphicsCommandList* commandList, uint32_t handle) {
	// Rtvのハンドルを取得
	DescriptorHandles rtvHandle = rtvHandles_.at(handle);
	// レンダーターゲットをクリア
	commandList->ClearRenderTargetView(rtvHandle.cpuHandle_, kClearColor, 0, nullptr);
}

bool QFE::GRAPHIC::INTERNAL::OffscreenBuffer::SetRenderTarget(ID3D12GraphicsCommandList* commandList, uint32_t handle) {
	return offscreens_.at(handle).TransitionResource(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

bool QFE::GRAPHIC::INTERNAL::OffscreenBuffer::SetTexture(ID3D12GraphicsCommandList* commandList, uint32_t handle) {
	return offscreens_.at(handle).TransitionResource(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

DescriptorHandles QFE::GRAPHIC::INTERNAL::OffscreenBuffer::GetRtvHandle(uint32_t handle) const {
	return rtvHandles_.at(handle);
}

const D3D12_CPU_DESCRIPTOR_HANDLE* QFE::GRAPHIC::INTERNAL::OffscreenBuffer::GetRtvHandlePtr(uint32_t handle) const {
	return &(rtvHandles_.at(handle).cpuHandle_);
}

DescriptorHandles QFE::GRAPHIC::INTERNAL::OffscreenBuffer::GetSrvHandle(uint32_t handle) const {
	return srvHandles_.at(handle);
}
