#include "CheckGenerateConfig.h"

bool CheckGenerateConfig::IsValid(const DescriptorGenerateConfig& config) {
	// heapTypeが有効な値であることを確認
	if (config.heapType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV &&
		config.heapType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV &&
		config.heapType != D3D12_DESCRIPTOR_HEAP_TYPE_DSV &&
		config.heapType != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
		return false;
	}
	// numDescriptorsが0より大きいことを確認
	if (config.numDescriptors == 0) {
		return false;
	}
	// descriptorSizeが0より大きいことを確認
	if (config.descriptorSize == 0) {
		return false;
	}
	// shaderVisibleがtrueまたはfalseであることを確認
	if (config.shaderVisible != true && config.shaderVisible != false) {
		return false;
	}
	return true; // すべてのチェックを通過した場合、設定は有効
}
