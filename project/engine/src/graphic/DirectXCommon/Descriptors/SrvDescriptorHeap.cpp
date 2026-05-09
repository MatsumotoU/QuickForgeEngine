#include "engine/include/graphic/DirectXCommon/Descriptors/SrvDescriptorHeap.h"

#include "engine/include/graphic/DirectXCommon/Descriptors/DescriptorGenerator/DescriptorGenerator.h"
#include "engine/include/graphic/DirectXCommon/Descriptors/CheckGenerateConfig/CheckGenerateConfig.h"
#include "engine/include/utility/DirectX/GenerateDescriptorHandle.h"

#include "engine/include/core/EngineDefines.h"

using namespace QFE;
void SrvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("-----SrvDescriptorHeap:Initialize-----");
#endif // QFE_OPTIMIZE_OFF
	// 繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち逕滓・險ｭ螳壹・蛻晄悄蛹・
	descriptorGenerateConfig_.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorGenerateConfig_.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorGenerateConfig_.numDescriptors = numDescriptors;
	descriptorGenerateConfig_.shaderVisible = shaderVisible;
	assert(CheckGenerateConfig::IsValid(descriptorGenerateConfig_));
	assert(device && "Device is null in SrvDescriptorHeap::Initialize");
	assert(numDescriptors > 0 && "Number of descriptors must be greater than zero.");
	assert(descriptorGenerateConfig_.descriptorSize > 0 && "Descriptor size must be greater than zero.");
	assert(descriptorGenerateConfig_.heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && "Heap type must be Srv for SrvDescriptorHeap.");
	assert(descriptorGenerateConfig_.shaderVisible == true && "Shader visibility must be true for Srv descriptor heap.");

	device_ = device; // 繝・ヰ繧､繧ｹ繧剃ｿ晏ｭ・

	// 繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝偵・繝励・逕滓・
	DescriptorGenerator::GenerateDescriptorHeap(
		descriptorHeap_, device, descriptorGenerateConfig_);
	// 遨ｺ縺阪せ繧ｿ繝・け繧貞・譛溷喧
	for (UINT i = 1; i < descriptorGenerateConfig_.numDescriptors; ++i) {
		freeDescriptors_.push(i);
	}
}

UINT SrvDescriptorHeap::GetDescriptorSize() const {
	return descriptorGenerateConfig_.descriptorSize;
}

DescriptorHandles SrvDescriptorHeap::AssignHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
	// 遨ｺ縺阪せ繧ｿ繝・け縺九ｉ繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繧貞叙蠕・
	assert(!freeDescriptors_.empty() && "No free descriptors available.");
	UINT index = freeDescriptors_.front();
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG(std::format("Srv_AssignHeapIndex: {}", index));
#endif // QFE_OPTIMIZE_OFF
	freeDescriptors_.pop();
	// 繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝上Φ繝峨Ν繧貞叙蠕・
	DescriptorHandles handle;
	handle.cpuHandle_ =
		GenerateDescriptorHandle::GetCpuDescriptorHandle(descriptorHeap_.Get(), descriptorGenerateConfig_.descriptorSize, index);
	handle.gpuHandle_ =
		GenerateDescriptorHandle::GetGpuDescriptorHandle(descriptorHeap_.Get(), descriptorGenerateConfig_.descriptorSize, index);

	// 繝ｪ繧ｽ繝ｼ繧ｹ繝薙Η繝ｼ繧堤函謌・
	device_->CreateShaderResourceView(resource, &desc, handle.cpuHandle_);
	return handle;
}


