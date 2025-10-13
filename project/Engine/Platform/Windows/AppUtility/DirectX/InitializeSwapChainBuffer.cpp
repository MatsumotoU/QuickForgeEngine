#include "InitializeSwapChainBuffer.h"

void InitializeSwapChainBuffer::Execute(ID3D12GraphicsCommandList* list,const D3D12_CPU_DESCRIPTOR_HANDLE& handle, float red, float green, float blue, float alpha) {
	float color[] = { red, green, blue, alpha };
	list->ClearRenderTargetView(handle, color, 0, nullptr);
}
