#include "engine/include/graphic/Pipeline/PSO/ShaderCompiler.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#include <cassert>
#include <format>

#include "engine/include/utility/FileSystems/FileUtility.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include "engine/include/graphic/Pipeline/PSO/ShaderReflection.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

ShaderCompiler::ShaderCompiler() {
	iDxcBlobMap_.clear();
}

ShaderCompiler::~ShaderCompiler() {
#ifdef _DEBUG
	DebugLog("=====ShaderFiles=====");
#endif // _DEBUG
	
	// iDxcBlobMap_縺ｫ譬ｼ邏阪＆繧後※縺・ｋIDxcBlob*繧偵☆縺ｹ縺ｦRelease縺励※縺九ｉ繧ｯ繝ｪ繧｢
	for (auto& [key, blob] : iDxcBlobMap_) {
		if (blob) {
			blob->Release();
#ifdef _DEBUG
			DebugLog(std::format("Delete: {}", ConvertString(key)));
#endif // _DEBUG
		}
	}
	iDxcBlobMap_.clear();
#ifdef _DEBUG
	DebugLog("=====================");
#endif // _DEBUG
}

void ShaderCompiler::InitializeDXC() {
	//// * DXC縺ｮ蛻晄悄蛹・* //
	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	// 迴ｾ譎らせ縺ｧinclude縺ｯ縺励↑縺・′縲（nclude縺ｫ蟇ｾ蠢懊☆繧九◆繧√・險ｭ螳壹ｒ陦後▲縺ｦ縺翫￥
	includeHandler_ = nullptr;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// 譌｢縺ｫ隱ｭ縺ｿ霎ｼ縺ｿ貂医∩縺ｮ繧ｷ繧ｧ繝ｼ繝繝ｼ繧貞・蠎ｦ隱ｭ縺ｿ霎ｼ縺ｾ縺ｪ縺・
	if (iDxcBlobMap_.contains(filePath)) {
#ifdef _DEBUG
		DebugLog(std::format("Loaded file: {}",ConvertString(filePath)));
#endif // _DEBUG
		return iDxcBlobMap_.at(filePath);
	}

	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));

	// ここで絶対パスを出力
	std::wstring absPath = QFE::FILE::GetAbsolutePath(filePath);
	Log(ConvertString(std::format(L"Shader file absolute path: {}\n", absPath)));

	// hlslファイルをロード
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 隱ｭ繧√↑縺・↑繧牙●豁｢
	assert(SUCCEEDED(hr));
	// 隱ｭ縺ｿ霎ｼ繧薙□繝輔ぃ繧､繝ｫ縺ｮ蜀・ｮｹ繧定ｨｭ螳壹☆繧・
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2:繧ｳ繝ｳ繝代う繝ｫ縺吶ｋ
	LPCWSTR arguments[] = {
		filePath.c_str(),		// 繧ｳ繝ｳ繝代う繝ｫ蟇ｾ雎｡縺ｮhlsl繝輔ぃ繧､繝ｫ蜷・
		L"-E",L"main",			// 繧ｨ繝ｳ繝医Μ繝ｼ繝昴う繝ｳ繝医・謖・ｮ壹ょ渕譛ｬmain莉･螟悶↓縺励↑縺・
		L"-T",profile,			// ShaderProfile縺ｮ險ｭ螳・
		L"-Zi",L"-Qembed_debug",// 繝・ヰ繝・げ逕ｨ縺ｮ諠・ｱ繧貞沂繧∬ｾｼ繧
		L"-Od",					// 譛驕ｩ蛹悶ｒ螟悶＠縺ｦ縺翫￥
		L"-Zpr",				// 繝ｬ繧､繧｢繧ｦ繝医・陦悟━蜈・
	};
	// 螳滄圀縺ｫ繧ｳ繝ｳ繝代う繝ｫ縺吶ｋ
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,		// 隱ｭ縺ｿ霎ｼ繧薙□繝輔ぃ繧､繝ｫ
		arguments,					// 繧ｳ繝ｳ繝代う繝ｫ繧ｪ繝励す繝ｧ繝ｳ
		_countof(arguments),		// 繧ｳ繝ｳ繝代う繝ｫ繧ｪ繝励す繝ｧ繝ｳ謨ｰ
		includeHandler_,				// include縺悟性縺ｾ繧後◆隲ｸ縲・
		IID_PPV_ARGS(&shaderResult)	// 繧ｳ繝ｳ繝代う繝ｫ邨先棡
	);
	assert(SUCCEEDED(hr));

	// 3:繧ｨ繝ｩ繝ｼ遒ｺ隱・
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	// 4:繧ｳ繝ｳ繝代う繝ｫ邨先棡繧貞女縺大叙縺｣縺ｦ霑斐☆
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"Compile Succeded, path:{},profile:{}\n", filePath, profile)));
	shaderSource->Release();
	shaderResult->Release();

	// 5:繧ｷ繧ｧ繝ｼ繝繝ｼ繝ｪ繝輔Ξ繧ｯ繧ｷ繝ｧ繝ｳ諠・ｱ繧貞､夜Κ縺ｫ蜃ｺ蜉帙☆繧・
	ShaderReflection shaderReflection;
	shaderReflection.RunShaderReflection(shaderBlob);
	nlohmann::json shaderJson = shaderReflection.Serialize();
	std::string savePath = "Resources/TestFolder/" + ConvertString(filePath.substr(filePath.find_last_of(L"/\\") + 1)) + ".json";
	QFE::FILE::SaveJSONToFile( savePath, shaderJson);

	// 繧ｷ繧ｧ繝ｼ繝繝ｼ繧堤匳骭ｲ
	iDxcBlobMap_.emplace(filePath, shaderBlob);
	return shaderBlob;
}
