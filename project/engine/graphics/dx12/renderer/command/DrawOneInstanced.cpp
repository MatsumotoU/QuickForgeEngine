#include "DrawOneInstanced.h"
#include <unordered_map>

#include "graphics/dx12/vram/descriptors/ViewTypeFlags.h"

using namespace QFE::GRAPHIC;

namespace {
	std::unordered_map<ViewTypeFlags, D3D12_ROOT_PARAMETER_TYPE> viewTypeToRootParameterTypeMap = {
		{ ViewTypeFlags::ConstantBufferView, D3D12_ROOT_PARAMETER_TYPE_CBV },
		{ ViewTypeFlags::ShaderResourceView, D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE },
		{ ViewTypeFlags::UnorderedAccessView, D3D12_ROOT_PARAMETER_TYPE_UAV },
	};

	ViewTypeFlags RootParameterTypeToViewType(D3D12_ROOT_PARAMETER_TYPE rootParamType) {
		switch (rootParamType) {
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
			return ViewTypeFlags::ConstantBufferView;
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: // SRV
			return ViewTypeFlags::ShaderResourceView;
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
			return ViewTypeFlags::UnorderedAccessView;
		default:
			return ViewTypeFlags::None;
		}
	}
}

DrawOneInstanced::DrawOneInstanced(DrawOneInstancedDesc desc) :
	vertexbufferHandle_(desc.vertexbufferHandle), psoHandle_(desc.psoHandle),
	signatureResourceHandles_(desc.signatureResourceHandles),
	getGpuAddressFunc_(desc.getGpuAddressFunc),
	getCpuAddressFunc_(desc.getCpuAddressFunc) {
	// PSOハンドルを元にルートパラメータのタイプを取得して保存
	rootParameterTypes_ = desc.getRootParameterTypesFunc(psoHandle_);
}

void DrawOneInstanced::Execute(ID3D12GraphicsCommandList* commandList) {
	// シグネチャに登録すべくリソースのGPUディスクリプタハンドルを取得し、ルートパラメータのタイプに応じてコマンドリストにセット
	for (size_t i = 0; i < signatureResourceHandles_.size(); ++i) {
		
		ViewTypeFlags viewType = RootParameterTypeToViewType(rootParameterTypes_[i]);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = getGpuAddressFunc_(signatureResourceHandles_[i], viewType);

		switch (rootParameterTypes_[i]) {
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle.ptr);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
			commandList->SetGraphicsRootShaderResourceView(static_cast<UINT>(i), gpuHandle.ptr);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
			commandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(i), gpuHandle.ptr);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
			break;
		default:
			break;
		}
	}



}
