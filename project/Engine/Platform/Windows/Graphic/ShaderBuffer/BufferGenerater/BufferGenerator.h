#pragma once
#include <d3d12.h>
#include <wrl.h>

class BufferGenerator final{
public:
	[[nodiscard]] static Microsoft::WRL::ComPtr<ID3D12Resource> Generate(ID3D12Device* device, size_t sizeInBytes);
};