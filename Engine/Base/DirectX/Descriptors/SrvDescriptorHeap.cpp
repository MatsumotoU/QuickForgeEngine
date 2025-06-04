#include "SrvDescriptorHeap.h"
#include "../DirectXCommon.h"

#ifdef _DEBUG
#include "../../MyDebugLog.h"
#include "../ImGuiManager.h"
#endif // _DEBUG

// TODO: メモリ領域の可視化
// TODO: メモリ選定機能の追加、外部から変更できないようにする

void SrvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	srvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescriptors, shaderVisible);
	DebugLog(std::format("CreateSRVDescriptorHeap: Create {} HeapSpace", numDescriptors));
}

#ifdef _DEBUG
void SrvDescriptorHeap::DrawDebugWindow() {
	ImGui::Begin("SrvDescriptorHeap");
	ImGui::End();
}
#endif // _DEBUG

ID3D12DescriptorHeap* SrvDescriptorHeap::GetSrvDescriptorHeap() {
	return srvDescriptorHeap_.Get();
}
