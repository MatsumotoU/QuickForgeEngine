#include "engine/include/graphic/Pipeline/PSO/ShaderReflection.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include <cassert>

ShaderReflection::ShaderReflection() {
	HRESULT hr;
	hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(containerReflection_.GetAddressOf()));
	assert(SUCCEEDED(hr) && "Failed to create DxcContainerReflection instance.");
}

void ShaderReflection::RunShaderReflection(IDxcBlob* shaderBlob) {
	HRESULT hr;
	hr = containerReflection_->Load(shaderBlob);
	assert(SUCCEEDED(hr) && "Failed to load shader blob into container reflection.");
	UINT32 partIndex;
	hr = containerReflection_->FindFirstPartKind(DXC_PART_DXIL, &partIndex);
	assert(SUCCEEDED(hr) && "Failed to find DXIL part in shader blob.");
	hr = containerReflection_->GetPartReflection(partIndex, IID_PPV_ARGS(shaderReflection_.GetAddressOf()));
	assert(SUCCEEDED(hr) && "Failed to get shader reflection from part.");

#ifdef QFE_OPTIMIZE_OFF
	
#endif // QFE_OPTIMIZE_OFF

}

nlohmann::json ShaderReflection::Serialize() const {
	nlohmann::json jsonData;
	try
	{
		// 繧ｷ繧ｧ繝ｼ繝繝ｼ縺ｮ隱ｬ譏弱ｒ蜿門ｾ・
		D3D12_SHADER_DESC shaderDesc{};
		HRESULT hr = shaderReflection_->GetDesc(&shaderDesc);
		assert(SUCCEEDED(hr) && "Failed to get shader description.");
		
		// InputLayout繧貞叙蠕・
		jsonData["Inputs"] = nlohmann::json::array();
		for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
			D3D12_SIGNATURE_PARAMETER_DESC paramDesc{};
			hr = shaderReflection_->GetInputParameterDesc(i, &paramDesc);
			assert(SUCCEEDED(hr) && "Failed to get input parameter description.");
			nlohmann::json inputJson;
			inputJson["SemanticName"] = paramDesc.SemanticName;
			inputJson["SemanticIndex"] = paramDesc.SemanticIndex;
			inputJson["Register"] = paramDesc.Register;
			inputJson["SystemValueType"] = paramDesc.SystemValueType;
			inputJson["ComponentType"] = paramDesc.ComponentType;
			inputJson["Mask"] = paramDesc.Mask;
			inputJson["ReadWriteMask"] = paramDesc.ReadWriteMask;
			jsonData["Inputs"].push_back(inputJson);
		}
		// 繝ｪ繧ｽ繝ｼ繧ｹ繝舌う繝ｳ繝・ぅ繝ｳ繧ｰ諠・ｱ繧貞叙蠕・
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
		// 螳壽焚繝舌ャ繝輔ぃ諠・ｱ繧貞叙蠕・
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
		// 繧ｹ繝医Λ繧ｯ繝√Ε繝ｼ繝峨ヰ繝・ヵ繧｡諠・ｱ繧貞叙蠕・
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
		// 繝・け繧ｹ繝√Ε諠・ｱ繧貞叙蠕・
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
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("ShaderReflection::Serialize: Exception occurred - " + std::string(e.what()));
#endif // =DEBUG

	}
	return jsonData;
}


