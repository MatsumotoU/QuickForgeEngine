#include "DepthStencil.h"
#include "DirectXCommon.h"
#include "../Windows/WinApp.h"

#include <cassert>

void DepthStencil::Initialize(WinApp* winApp, DirectXCommon* dxCommon) {
    dxCommon_ =dxCommon;
    dsvDescriptorHeap_ = dxCommon->CreateDescriptorHeap(dxCommon->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    depthStencilResource_ = CreateDepthStencilTextureResource(dxCommon->GetDevice(), winApp->kWindowWidth, winApp->kWindowHeight);

    dsvDesc_ = {};
    dsvDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc_.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dxCommon->GetDevice()->CreateDepthStencilView(depthStencilResource_, &dsvDesc_, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());

    depthStencilDesc_ = {};
    depthStencilDesc_.DepthEnable = true;
    depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

ID3D12Resource* DepthStencil::CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
    resourceDesc_ = {};
    resourceDesc_.Width = width;
    resourceDesc_.Height = height;
    resourceDesc_.MipLevels = 1;
    resourceDesc_.DepthOrArraySize = 1;
    resourceDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    resourceDesc_.SampleDesc.Count = 1;
    resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc_.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    // 深度値のクリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    ID3D12Resource* resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc_,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(hr));
    return resource;
}

D3D12_DEPTH_STENCIL_DESC* DepthStencil::GetDepthStencilDesc() {
    return &depthStencilDesc_;
}

ID3D12DescriptorHeap* DepthStencil::GetDsvDescriptorHeap() {
    return dsvDescriptorHeap_;
}
