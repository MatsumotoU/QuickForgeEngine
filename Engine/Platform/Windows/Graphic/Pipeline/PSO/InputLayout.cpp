#include "InputLayout.h"

void InputLayout::Initialize() {
	inputElementDescs_.clear();
	inputLayoutDesc_ = {};
}

void InputLayout::CreateNormalPresetInputLayout() {
	Initialize();
	CreateInputElementDesc("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
	CreateInputElementDesc("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
	CreateInputElementDesc("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
}

void InputLayout::CreatePrimitivePresetInputLayout() {
	Initialize();
	CreateInputElementDesc("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
	CreateInputElementDesc("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
	CreateInputElementDesc("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
}

void InputLayout::CreateInputElementDesc(const LPCSTR& semanticName, const UINT& semanticIndex, const DXGI_FORMAT& format, const UINT& alignedByteOffset) {
	D3D12_INPUT_ELEMENT_DESC inputElementDescs{};
	inputElementDescs_.push_back(inputElementDescs);
	int32_t index = static_cast<int32_t>(inputElementDescs_.size()) - 1;
	inputElementDescs_[index].SemanticName = semanticName;
	inputElementDescs_[index].SemanticIndex = semanticIndex;
	inputElementDescs_[index].Format = format;
	inputElementDescs_[index].AlignedByteOffset = alignedByteOffset;

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_.data();
	inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescs_.size());
}

D3D12_INPUT_LAYOUT_DESC* InputLayout::GetInputLayoutDesc() {
	return &inputLayoutDesc_;
}
