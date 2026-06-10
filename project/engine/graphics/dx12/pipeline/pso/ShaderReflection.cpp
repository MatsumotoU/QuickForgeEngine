#include "ShaderReflection.h"

#include "EngineDefines.h"

#include <cassert>
#include <bitset>
using namespace QFE::GRAPHIC::INTERNAL;

namespace {
	uint32_t GetBitCount(BYTE mask) {
		if(mask && 0x1) {
			return 0;
		} else if(mask && 0x3) {
			return 1;
		} else if(mask && 0x7) {
			return 2;
		} else if(mask && 0xf) {
			return 3;
		}
		return UINT32_MAX;
	};

	const DXGI_FORMAT FormatTable[4][4] = {
		{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN}, // UNKNOWN
		{ DXGI_FORMAT_R32_UINT,  DXGI_FORMAT_R32G32_UINT,  DXGI_FORMAT_R32G32B32_UINT,  DXGI_FORMAT_R32G32B32A32_UINT  }, // UINT
		{ DXGI_FORMAT_R32_SINT,  DXGI_FORMAT_R32G32_SINT,  DXGI_FORMAT_R32G32B32_SINT,  DXGI_FORMAT_R32G32B32A32_SINT  }, // SINT
		{ DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT }, // FLOAT
	};
}


ShaderReflection::ShaderReflection() {
	HRESULT hr;
	hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(containerReflection_.GetAddressOf()));
	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create DxcContainerReflection instance.",SystemError::Abort);
	}
}

void ShaderReflection::RunShaderReflection(IDxcBlob* shaderBlob) {
	HRESULT hr;
	hr = containerReflection_->Load(shaderBlob);
	if(SUCCEEDED(hr)) {
		QFE_LOG("Shader blob loaded successfully into container reflection.");
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to load shader blob into container reflection.", SystemError::Abort);
	}
	UINT32 partIndex;
	hr = containerReflection_->FindFirstPartKind(DXC_PART_DXIL, &partIndex);
	if(SUCCEEDED(hr)) {
		QFE_LOG(std::format("DXIL part found in container at index {}.", partIndex));
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to find DXIL part in container reflection.", SystemError::Abort);
	}
	hr = containerReflection_->GetPartReflection(partIndex, IID_PPV_ARGS(shaderReflection_.GetAddressOf()));
	if(SUCCEEDED(hr)) {
		QFE_LOG("Shader reflection interface obtained successfully.");
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to get shader reflection interface from container reflection.", SystemError::Abort);
	}
}

std::vector<InputElement> QFE::GRAPHIC::INTERNAL::ShaderReflection::GetInputLayoutElement() const {
	std::vector<InputElement> inputLayoutElements;

	// shaderReflection_が有効かどうかを確認
	D3D12_SHADER_DESC shaderDesc{};
	if (shaderReflection_ == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Shader reflection interface is not initialized.", SystemError::Abort);
	}
	// シェーダーの基本情報を取得
	HRESULT hr = shaderReflection_->GetDesc(&shaderDesc);
	assert(SUCCEEDED(hr) && "Failed to get shader description.");
	// 入力パラメータの情報を取得して渡された引数に設定
	for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
		InputElement element{};
		D3D12_SIGNATURE_PARAMETER_DESC paramDesc{};
		hr = shaderReflection_->GetInputParameterDesc(i, &paramDesc);
		assert(SUCCEEDED(hr) && "Failed to get input parameter description.");
		element.semanticName = paramDesc.SemanticName;
		element.semanticIndex = paramDesc.SemanticIndex;
		element.format = FormatTable[paramDesc.ComponentType][GetBitCount(paramDesc.Mask)];
		element.alignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;// 入力要素のオフセットは自動で計算させるため、D3D12_APPEND_ALIGNED_ELEMENTを使用
		inputLayoutElements.push_back(element);
	}

	return inputLayoutElements;
}

std::vector<RootParameterElement> QFE::GRAPHIC::INTERNAL::ShaderReflection::GetRootParameterElement() const {
	std::vector<RootParameterElement> rootParameterElements;

	// shaderReflection_が有効かどうかを確認
	D3D12_SHADER_DESC shaderDesc{};
	if (shaderReflection_ == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Shader reflection interface is not initialized.", SystemError::Abort);
	}
	// シェーダーの基本情報を取得
	HRESULT hr = shaderReflection_->GetDesc(&shaderDesc);
	assert(SUCCEEDED(hr) && "Failed to get shader description.");
	// バウンドリソースの情報を取得して渡された引数に設定
	for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
		RootParameterElement element;
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		hr = shaderReflection_->GetResourceBindingDesc(i, &bindDesc);
		assert(SUCCEEDED(hr) && "Failed to get resource binding description.");
		element.friendlyName = bindDesc.Name;
		element.shaderInputType = bindDesc.Type;
		element.shaderRegisterIndex = bindDesc.BindPoint;
		rootParameterElements.push_back(element);
	}

	return rootParameterElements;
}