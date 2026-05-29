#include"OffScreenResourceManager.h"
#include <assert.h>
using namespace QFE;
void OffScreenResourceManager::Initialize(ID3D12Device* device, int width, int height) {
	offScreenDesc_ = {};
	offScreenDesc_.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	offScreenDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	offScreenDesc_.Width = width;
	offScreenDesc_.Height = height;
	offScreenDesc_.DepthOrArraySize = 1; // 繝・け繧ｹ繝√Ε縺ｪ縺ｮ縺ｧ騾壼ｸｸ縺ｯ1
	offScreenDesc_.MipLevels = 1;       // 騾壼ｸｸ縺ｯ1
	offScreenDesc_.SampleDesc.Count = 1;
	offScreenDesc_.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // 繝ｬ繝ｳ繝繝ｼ繧ｿ繝ｼ繧ｲ繝・ヨ縺ｨ縺励※菴ｿ逕ｨ

	offscreenClearValue_ = {};
	offscreenClearValue_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	offscreenClearValue_.Color[0] = 0.0f;
	offscreenClearValue_.Color[1] = 0.0f;
	offscreenClearValue_.Color[2] = 0.0f;
	offscreenClearValue_.Color[3] = 0.0f;

	D3D12_HEAP_PROPERTIES defaultHeapProperties{};
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	// 繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ逕滓・
	HRESULT hr = device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&offScreenDesc_,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 蛻晄悄迥ｶ諷・
		&offscreenClearValue_,
		IID_PPV_ARGS(offScreenResource_.at(0).GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));

	// 莠後▽逶ｮ縺ｮ繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ繝ｪ繧ｽ繝ｼ繧ｹ繧剃ｽ懈・
	hr = device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&offScreenDesc_,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 蛻晄悄迥ｶ諷・
		&offscreenClearValue_,
		IID_PPV_ARGS(offScreenResource_.at(1).GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));
}

void OffScreenResourceManager::ClearRenderTarget(ID3D12GraphicsCommandList* commandList, int index) {
	assert(index >= 0 && index < kOffScreenCount_);
	commandList->ClearRenderTargetView(offScreenRtvHandle_.at(index), offscreenClearValue_.Color, 0, nullptr);
}

ID3D12Resource* OffScreenResourceManager::GetOffscreenResource(int index) {
	return offScreenResource_.at(index).Get();
}

uint32_t OffScreenResourceManager::GetOffscreenCount() const {
	return kOffScreenCount_;
}

D3D12_SHADER_RESOURCE_VIEW_DESC OffScreenResourceManager::GetOffscreenSrvDesc() {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	return srvDesc;
}

D3D12_CPU_DESCRIPTOR_HANDLE OffScreenResourceManager::GetOffscreenRtvHandles(int index) {
	return offScreenRtvHandle_.at(index);
}

DescriptorHandles OffScreenResourceManager::GetOffscreenSrvHandles(int index) {
	return offScreenSrvHandles_.at(index);
}

void OffScreenResourceManager::SetRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle, int index) {
	offScreenRtvHandle_.at(index) = handle;
}

void OffScreenResourceManager::SetSrvHandle(DescriptorHandles handles, int index) {
	offScreenSrvHandles_.at(index) = handles;
}
