#include "DescriptorHeapInfo.h"

using namespace QFE::GRAPHIC;

bool DescriptorHeapInfo::CheckValid() const {
	// heapTypeが有効な値であることを確認
	if (heapType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV &&
		heapType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV &&
		heapType != D3D12_DESCRIPTOR_HEAP_TYPE_DSV &&
		heapType != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
		return false;
	}
	// numDescriptorsが0より大きいことを確認
	if (numDescriptors == 0) {
		return false;
	}
	// descriptorSizeが0より大きいことを確認
	if (descriptorSize == 0) {
		return false;
	}
	// shaderVisibleがtrueまたはfalseであることを確認
	if (shaderVisible != true && shaderVisible != false) {
		return false;
	}
	return true; // すべてのチェックを通過した場合、設定は有効
}
