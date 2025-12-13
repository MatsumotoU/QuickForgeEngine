#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <wrl.h>
#include <nlohmann/json.hpp>

class ShaderReflection final {
public:
	ShaderReflection();
	~ShaderReflection() = default;
public:
	void RunShaderReflection(IDxcBlob* shaderBlob);
	nlohmann::json Serialize() const;

private:
	Microsoft::WRL::ComPtr<IDxcContainerReflection> containerReflection_;
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection_;
};
