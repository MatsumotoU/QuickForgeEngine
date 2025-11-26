#pragma once
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <wrl.h>

struct ShaderCompiledData {
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection;
};