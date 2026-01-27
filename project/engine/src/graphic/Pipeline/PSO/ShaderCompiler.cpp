#include "engine/include/graphic/Pipeline/PSO/ShaderCompiler.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#include <cassert>
#include <format>

#include "engine/include/utility/FileSystems/FileUtility.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include "engine/include/graphic/Pipeline/PSO/ShaderReflection.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
using namespace QFE;
ShaderCompiler::ShaderCompiler() {
	iDxcBlobMap_.clear();
}

ShaderCompiler::~ShaderCompiler() {
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("=====ShaderFiles=====");
#endif // QFE_OPTIMIZE_OFF

	// iDxcBlobMap_縺ｫ譬ｼ邏阪＆繧後※縺・ｋIDxcBlob*繧坦elease縺励※隗｣謾ｾ
	for (auto& [key, blob] : iDxcBlobMap_) {
		if (blob) {
			blob->Release();
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::format("Delete: {}", ConvertString(key)));
#endif // QFE_OPTIMIZE_OFF
		}
	}
	iDxcBlobMap_.clear();
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("=====================");
#endif // QFE_OPTIMIZE_OFF
}

void ShaderCompiler::InitializeDXC() {
	//// * DXC縺ｮ蛻晄悄蛹・* //
	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	// 繧､繝ｳ繧ｯ繝ｫ繝ｼ繝牙ｯｾ蠢懊・縺溘ａ縺ｮ繝上Φ繝峨Λ菴懈・
	includeHandler_ = nullptr;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// 縺吶〒縺ｫ繧ｳ繝ｳ繝代う繝ｫ貂医∩縺ｪ繧峨く繝｣繝・す繝･縺九ｉ蜿門ｾ・
	if (iDxcBlobMap_.contains(filePath)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::format("Loaded file: {}", ConvertString(filePath)));
#endif // QFE_OPTIMIZE_OFF
		return iDxcBlobMap_.at(filePath);
	}

	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));

	// 邨ｶ蟇ｾ繝代せ繧貞・蜉・
	std::wstring absPath = QFE::FILE::GetAbsolutePath(filePath);
	Log(ConvertString(std::format(L"Shader file absolute path: {}\n", absPath)));

	// hlsl繝輔ぃ繧､繝ｫ繧偵Ο繝ｼ繝・
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));
	// 繝舌ャ繝輔ぃ菴懈・
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 繧ｳ繝ｳ繝代う繝ｫ蠑墓焚
	LPCWSTR arguments[] = {
		filePath.c_str(),		// 蜈･蜉嬋lsl繝輔ぃ繧､繝ｫ
		L"-E",L"main",			// 繧ｨ繝ｳ繝医Μ繝ｼ繝昴う繝ｳ繝・ain
		L"-T",profile,			// ShaderProfile
		L"-Zi",L"-Qembed_debug",// 繝・ヰ繝・げ諠・ｱ蝓九ａ霎ｼ縺ｿ
		L"-Od",					// 譛驕ｩ蛹也┌蜉ｹ
		L"-Zpr",				// 陦悟━蜈医ヱ繝・く繝ｳ繧ｰ
	};
	// 繧ｷ繧ｧ繝ｼ繝繝ｼ繧偵さ繝ｳ繝代う繝ｫ
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,		// 蜈･蜉帙ヵ繧｡繧､繝ｫ
		arguments,					// 蠑墓焚
		_countof(arguments),		// 蠑墓焚謨ｰ
		includeHandler_,			// 繧､繝ｳ繧ｯ繝ｫ繝ｼ繝峨ワ繝ｳ繝峨Λ
		IID_PPV_ARGS(&shaderResult)	// 邨先棡
	);
	assert(SUCCEEDED(hr));

	// 繧ｨ繝ｩ繝ｼ蜃ｺ蜉帙ｒ蜿門ｾ・
	IDxcBlobUtf8* shaderError = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (SUCCEEDED(hr) && shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	// 繝舌う繝翫Μ蜃ｺ蜉帙ｒ蜿門ｾ・
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"Compile Succeded, path:{},profile:{}\n", filePath, profile)));
	shaderSource->Release();
	shaderResult->Release();

	// 繧ｷ繧ｧ繝ｼ繝繝ｼ繝ｪ繝輔Ξ繧ｯ繧ｷ繝ｧ繝ｳ繧貞ｮ溯｡後＠JSON菫晏ｭ・
	ShaderReflection shaderReflection;
	shaderReflection.RunShaderReflection(shaderBlob);
	nlohmann::json shaderJson = shaderReflection.Serialize();
	std::string savePath = "Resources/ShaderReflection/" + ConvertString(filePath.substr(filePath.find_last_of(L"/\\") + 1)) + ".json";
	QFE::FILE::SaveJSONToFile(savePath, shaderJson);

	// 繧ｭ繝｣繝・す繝･縺ｫ菫晏ｭ・
	iDxcBlobMap_.emplace(filePath, shaderBlob);
	return shaderBlob;
}

nlohmann::json ShaderCompiler::GetShaderReflectionJson(const std::wstring& filePath) const {
	if (!iDxcBlobMap_.contains(filePath)) {
		Log(ConvertString(std::format(L"Shader not compiled yet: {}", filePath)));
		return nlohmann::json();
	}
	IDxcBlob* blob = iDxcBlobMap_.at(filePath);
	ShaderReflection shaderReflection;
	shaderReflection.RunShaderReflection(blob);
	nlohmann::json shaderJson = shaderReflection.Serialize();
	return shaderJson;
}

std::map<std::string, nlohmann::json> ShaderCompiler::GetAllShaderReflectionJson() const {
	std::map<std::string, nlohmann::json> shaderJsonMap;
	for (const auto& [filePath, blob] : iDxcBlobMap_) {
		ShaderReflection shaderReflection;
		shaderReflection.RunShaderReflection(blob);
		nlohmann::json shaderJson = shaderReflection.Serialize();
		shaderJsonMap[ConvertString(filePath)] = shaderJson;
	}
	return shaderJsonMap;
}


