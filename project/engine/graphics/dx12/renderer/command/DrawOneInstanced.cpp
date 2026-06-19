#include "DrawOneInstanced.h"
#include <unordered_map>

namespace {
	std::unordered_map<INTERNAL::ViewTypeFlags, D3D12_ROOT_PARAMETER_TYPE> viewTypeToRootParameterTypeMap = {
		{ INTERNAL::ViewTypeFlags::ConstantBufferView, D3D12_ROOT_PARAMETER_TYPE_CBV },
		{ INTERNAL::ViewTypeFlags::ShaderResourceView, D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE },
		{ INTERNAL::ViewTypeFlags::UnorderedAccessView, D3D12_ROOT_PARAMETER_TYPE_UAV },
	};
}

using namespace QFE::GRAPHIC;

DrawOneInstanced::DrawOneInstanced(DrawOneInstancedDesc desc) :
	vertexbufferHandle_(desc.vertexbufferHandle), psoHandle_(desc.psoHandle),
	signatureResourceHandles_(desc.signatureResourceHandles),
	getGpuAddressFunc_(desc.getGpuAddressFunc),
	getCpuAddressFunc_(desc.getCpuAddressFunc) {
	// PSOハンドルを元にルートパラメータのタイプを取得して保存
	rootParameterTypes_ = desc.getRootParameterTypesFunc(psoHandle_);
}

void DrawOneInstanced::Execute(ID3D12GraphicsCommandList* commandList) {

	

	//// シグネチャに登録すべくリソースのGPUディスクリプタハンドルを取得し、ルートパラメータのタイプに応じてコマンドリストにセット
	//for (size_t i = 0; i < signatureResourceHandles_.size(); ++i) {
	//	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = getGpuAddressFunc_(signatureResourceHandles_[i], rootParameterTypes_[i]);
	//	switch (rootParameterTypes_[i]) {
	//	case D3D12_ROOT_PARAMETER_TYPE_CBV:
	//		commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle.ptr);
	//		break;
	//	case D3D12_ROOT_PARAMETER_TYPE_SRV:
	//		commandList->SetGraphicsRootShaderResourceView(static_cast<UINT>(i), gpuHandle.ptr);
	//		break;
	//	case D3D12_ROOT_PARAMETER_TYPE_UAV:
	//		commandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(i), gpuHandle.ptr);
	//		break;
	//	case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
	//		commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
	//		break;
	//	default:
	//		QFE_REPORT_SYSTEM_ERROR("Unsupported root parameter type in DrawOneInstanced::Execute", SystemError::Abort);
	//		break;
	//	}
	//}

}
