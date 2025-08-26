#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "../Data/DescriptorGenerateConfig.h"

class DescriptorGenerator final{
public:
	static void GenerateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap, ID3D12Device* device,const DescriptorGenerateConfig& config);
};