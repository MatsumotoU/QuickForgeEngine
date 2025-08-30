#pragma once
#include <d3d12.h>
#include <array>
#include <memory>
#include <wrl.h>

#include "Graphic/DirectXCommon/Descriptors/Data/DescriptorHandles.h"

class OffScreenResourceManager {
public:
	void Initialize(ID3D12Device* device, int width, int height);
	void ClearRenderTarget(ID3D12GraphicsCommandList* commandList, int index);

	ID3D12Resource* GetOffscreenResource(int index);
	uint32_t GetOffscreenCount() const;
	D3D12_SHADER_RESOURCE_VIEW_DESC GetOffscreenSrvDesc();
	D3D12_CPU_DESCRIPTOR_HANDLE GetOffscreenRtvHandles(int index);
	DescriptorHandles GetOffscreenSrvHandles(int index);
	void SetRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle, int index);
	void SetSrvHandle(DescriptorHandles handles, int index);
	
private:
	static inline const uint32_t kOffScreenCount_ = 2;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kOffScreenCount_> offScreenResource_;
	D3D12_RESOURCE_DESC offScreenDesc_;
	D3D12_CLEAR_VALUE offscreenClearValue_;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kOffScreenCount_> offScreenRtvHandle_;
	std::array<DescriptorHandles, kOffScreenCount_> offScreenSrvHandles_;
};