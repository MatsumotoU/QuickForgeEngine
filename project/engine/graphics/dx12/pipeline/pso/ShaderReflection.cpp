#include "ShaderReflection.h"

#include "EngineDefines.h"

#include <cassert>
#include <bitset>
using namespace QFE::GRAPHIC;

namespace {
	uint32_t GetBitCount(BYTE mask) {
		if (mask == 0x1) {
			return 0; // 1成分 (例: float)
		} else if (mask == 0x3) {
			return 1; // 2成分 (例: float2)
		} else if (mask == 0x7) {
			return 2; // 3成分 (例: float3)
		} else if (mask == 0xf) {
			return 3; // 4成分 (例: float4)
		}
		return UINT32_MAX;
	}

	const DXGI_FORMAT FormatTable[4][4] = {
		{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN}, // UNKNOWN
		{ DXGI_FORMAT_R32_UINT,  DXGI_FORMAT_R32G32_UINT,  DXGI_FORMAT_R32G32B32_UINT,  DXGI_FORMAT_R32G32B32A32_UINT  }, // UINT
		{ DXGI_FORMAT_R32_SINT,  DXGI_FORMAT_R32G32_SINT,  DXGI_FORMAT_R32G32B32_SINT,  DXGI_FORMAT_R32G32B32A32_SINT  }, // SINT
		{ DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT }, // FLOAT
	};
}

std::vector<InputElement> QFE::GRAPHIC::ShaderReflection::GetInputLayoutElement(IDxcBlob* shaderBlob) {
	std::vector<InputElement> inputLayoutElements;

	// shaderBlobが有効かどうかを確認
	if (shaderBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Shader blob is null.", SystemError::Abort);
	}
	

	// shaderReflection_が有効かどうかを確認
	D3D12_SHADER_DESC shaderDesc{};
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection = CreateShaderReflection(shaderBlob);

	// シェーダーの基本情報を取得
	HRESULT hr = shaderReflection->GetDesc(&shaderDesc);
	assert(SUCCEEDED(hr) && "Failed to get shader description.");
	// 入力パラメータの情報を取得して渡された引数に設定
	for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
		InputElement element{};
		D3D12_SIGNATURE_PARAMETER_DESC paramDesc{};
		hr = shaderReflection->GetInputParameterDesc(i, &paramDesc);
		assert(SUCCEEDED(hr) && "Failed to get input parameter description.");
		element.semanticName = paramDesc.SemanticName;
		element.semanticIndex = paramDesc.SemanticIndex;
		element.format = FormatTable[paramDesc.ComponentType][GetBitCount(paramDesc.Mask)];
		element.alignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputLayoutElements.push_back(element);
	}

	return inputLayoutElements;
}

std::vector<RootParameterElement> QFE::GRAPHIC::ShaderReflection::GetRootParameterElement(IDxcBlob* shaderBlob) {
	std::vector<RootParameterElement> rootParameterElements;
	D3D12_SHADER_DESC shaderDesc{};

	// shaderBlobが有効かどうかを確認
	if (shaderBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Shader blob is null.", SystemError::Abort);
	}

	// シェーダーのリフレクションを実行
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection = CreateShaderReflection(shaderBlob);
	// シェーダーの基本情報を取得
	HRESULT hr = shaderReflection->GetDesc(&shaderDesc);
	assert(SUCCEEDED(hr) && "Failed to get shader description.");
	// バウンドリソースの情報を取得して渡された引数に設定
	for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
		RootParameterElement element;
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		hr = shaderReflection->GetResourceBindingDesc(i, &bindDesc);
		assert(SUCCEEDED(hr) && "Failed to get resource binding description.");
		element.friendlyName = bindDesc.Name;
		element.shaderInputType = bindDesc.Type;
		element.shaderRegisterIndex = bindDesc.BindPoint;
		rootParameterElements.push_back(element);
	}

	return rootParameterElements;
}

bool QFE::GRAPHIC::ShaderReflection::GetThreadGroupSize(IDxcBlob* shaderBlob, UINT& sizeX, UINT& sizeY, UINT& sizeZ) {
	// shaderBlobが有効かどうかを確認
	if (shaderBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Shader blob is null.", SystemError::Abort);
	}

	// シェーダーのリフレクションを実行
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection = CreateShaderReflection(shaderBlob);
	// スレッドグループサイズを取得
	shaderReflection->GetThreadGroupSize(&sizeX, &sizeY, &sizeZ);
	return true;
}

UINT QFE::GRAPHIC::ShaderReflection::GetRenderTargetCount(IDxcBlob* shaderBlob) {
	// shaderBlobが有効かどうかを確認
	if (shaderBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Shader blob is null.", SystemError::Abort);
	}

	// シェーダーのリフレクションを実行
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection = CreateShaderReflection(shaderBlob);
	// シェーダーの基本情報を取得
	D3D12_SHADER_DESC shaderDesc{};
	HRESULT hr = shaderReflection->GetDesc(&shaderDesc);
	assert(SUCCEEDED(hr) && "Failed to get shader description.");

	QFE_LOG(std::format("Shader has {} render target(s).", shaderDesc.OutputParameters));
	return shaderDesc.OutputParameters;
}

Microsoft::WRL::ComPtr<IDxcContainerReflection> QFE::GRAPHIC::ShaderReflection::CreateContainerReflection(IDxcBlob* shaderBlob) {
	HRESULT hr;
	Microsoft::WRL::ComPtr<IDxcContainerReflection> containerReflection;
	// IDxcContainerReflectionのインスタンスを作成
	hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(containerReflection.GetAddressOf()));
	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create DxcContainerReflection instance.", SystemError::Abort);
	}
	return containerReflection;
}

Microsoft::WRL::ComPtr<ID3D12ShaderReflection> QFE::GRAPHIC::ShaderReflection::CreateShaderReflection(IDxcBlob* shaderBlob) {
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection;

	// 使用する機能の生成
	Microsoft::WRL::ComPtr<IDxcContainerReflection> containerReflection = CreateContainerReflection(shaderBlob);

	// シェーダーバイナリをコンテナにロード
	HRESULT hr = containerReflection->Load(shaderBlob);
	if (SUCCEEDED(hr)) {
		QFE_LOG("Shader blob loaded successfully into container reflection.");
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to load shader blob into container reflection.", SystemError::Abort);
	}
	// リフレクションの情報の先頭の場所をコンテナから取得
	UINT32 partIndex;
	hr = containerReflection->FindFirstPartKind(DXC_PART_DXIL, &partIndex);
	if (SUCCEEDED(hr)) {
		QFE_LOG(std::format("DXIL part found in container at index {}.", partIndex));
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to find DXIL part in container reflection.", SystemError::Abort);
	}
	// シェーダーリフレクションのインターフェースをコンテナから取得
	hr = containerReflection->GetPartReflection(partIndex, IID_PPV_ARGS(shaderReflection.GetAddressOf()));
	if (SUCCEEDED(hr)) {
		QFE_LOG("Shader reflection interface obtained successfully.");
	} else {
		QFE_REPORT_SYSTEM_ERROR("Failed to get shader reflection interface from container reflection.", SystemError::Abort);
	}

	return shaderReflection;
}
