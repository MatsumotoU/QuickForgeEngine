#include "ShaderReflection.h"

#include "EngineDefines.h"

#include <cassert>
using namespace QFE::GRAPHIC::INTERNAL;
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

nlohmann::json ShaderReflection::Serialize() const {
	nlohmann::json jsonData;
	try
	{
		// shaderReflection_が有効かどうかを確認
		D3D12_SHADER_DESC shaderDesc{};
		if(shaderReflection_ == nullptr) {
			QFE_REPORT_SYSTEM_ERROR("Shader reflection interface is not initialized.", SystemError::Abort);
		}
		// シェーダーの基本情報を取得
		HRESULT hr = shaderReflection_->GetDesc(&shaderDesc);
		assert(SUCCEEDED(hr) && "Failed to get shader description.");
		
		// シェーダーの基本情報をJSONに追加
		jsonData["Inputs"] = nlohmann::json::array();
		for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
			D3D12_SIGNATURE_PARAMETER_DESC paramDesc{};
			hr = shaderReflection_->GetInputParameterDesc(i, &paramDesc);
			assert(SUCCEEDED(hr) && "Failed to get input parameter description.");
			nlohmann::json inputJson;
			inputJson["SemanticName"] = paramDesc.SemanticName;
			inputJson["SemanticIndex"] = paramDesc.SemanticIndex;
			inputJson["Mask"] = paramDesc.Mask;
			inputJson["ComponentType"] = paramDesc.ComponentType;
			jsonData["Inputs"].push_back(inputJson);
		}
		// シェーダーのリソース情報をJSONに追加
		jsonData["Resources"] = nlohmann::json::array();
		for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
			D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
			hr = shaderReflection_->GetResourceBindingDesc(i, &bindDesc);
			assert(SUCCEEDED(hr) && "Failed to get resource binding description.");
			nlohmann::json resourceJson;
			resourceJson["Name"] = bindDesc.Name;
			resourceJson["Type"] = bindDesc.Type;
			resourceJson["BindPoint"] = bindDesc.BindPoint;
			resourceJson["BindCount"] = bindDesc.BindCount;
			resourceJson["Space"] = bindDesc.Space;
			resourceJson["Flags"] = bindDesc.uFlags;
			jsonData["Resources"].push_back(resourceJson);
		}
		// 定数バッファの情報をJSONに追加
		jsonData["ConstantBuffers"] = nlohmann::json::array();
		for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i) {
			ID3D12ShaderReflectionConstantBuffer* constBuffer = shaderReflection_->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC bufferDesc{};
			hr = constBuffer->GetDesc(&bufferDesc);
			assert(SUCCEEDED(hr) && "Failed to get constant buffer description.");
			nlohmann::json bufferJson;
			bufferJson["Name"] = bufferDesc.Name;
			bufferJson["Size"] = bufferDesc.Size;
			bufferJson["Variables"] = nlohmann::json::array();
			for (UINT j = 0; j < bufferDesc.Variables; ++j) {
				ID3D12ShaderReflectionVariable* variable = constBuffer->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC varDesc{};
				hr = variable->GetDesc(&varDesc);
				assert(SUCCEEDED(hr) && "Failed to get variable description.");
				nlohmann::json varJson;
				varJson["Name"] = varDesc.Name;
				varJson["StartOffset"] = varDesc.StartOffset;
				varJson["Size"] = varDesc.Size;
				bufferJson["Variables"].push_back(varJson);
			}
			jsonData["ConstantBuffers"].push_back(bufferJson);
		}
		// 構造化バッファの情報をJSONに追加
		jsonData["StructuredBuffers"] = nlohmann::json::array();
		for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
			D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
			hr = shaderReflection_->GetResourceBindingDesc(i, &bindDesc);
			assert(SUCCEEDED(hr) && "Failed to get resource binding description.");
			if (bindDesc.Type == D3D_SIT_STRUCTURED) {
				nlohmann::json structuredBufferJson;
				structuredBufferJson["Name"] = bindDesc.Name;
				structuredBufferJson["BindPoint"] = bindDesc.BindPoint;
				structuredBufferJson["BindCount"] = bindDesc.BindCount;
				structuredBufferJson["Space"] = bindDesc.Space;
				structuredBufferJson["Flags"] = bindDesc.uFlags;
				jsonData["StructuredBuffers"].push_back(structuredBufferJson);
			}
		}
		// テクスチャの情報をJSONに追加
		jsonData["Textures"] = nlohmann::json::array();
		for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
			D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
			hr = shaderReflection_->GetResourceBindingDesc(i, &bindDesc);
			assert(SUCCEEDED(hr) && "Failed to get resource binding description.");
			if (bindDesc.Type == D3D_SIT_TEXTURE) {
				nlohmann::json textureJson;
				textureJson["Name"] = bindDesc.Name;
				textureJson["BindPoint"] = bindDesc.BindPoint;
				textureJson["BindCount"] = bindDesc.BindCount;
				textureJson["Space"] = bindDesc.Space;
				textureJson["Flags"] = bindDesc.uFlags;
				jsonData["Textures"].push_back(textureJson);
			}
		}
	}
	catch (const std::exception& e)
	{
		e;
		QFE_LOG("ShaderReflection::Serialize: Exception occurred - " + std::string(e.what()));
	}
	return jsonData;
}


