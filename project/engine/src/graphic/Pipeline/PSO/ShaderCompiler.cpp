#include "ShaderCompiler.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#include <cassert>

#include "engine/include/utility/FileSystems/FileUtility.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include "ShaderReflection.h"

ShaderCompiler::ShaderCompiler() {
	iDxcBlobMap_.clear();
}

ShaderCompiler::~ShaderCompiler() {
#ifdef _DEBUG
	DebugLog("=====ShaderFiles=====");
#endif // _DEBUG
	
	// iDxcBlobMap_に格納されてぁE��IDxcBlob*をすべてReleaseしてからクリア
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
	//// * DXCの初期匁E* //
	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしなぁE��、includeに対応するため�E設定を行っておく
	includeHandler_ = nullptr;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// 既に読み込み済みのシェーダーを�E度読み込まなぁE
	if (iDxcBlobMap_.contains(filePath)) {
#ifdef _DEBUG
		DebugLog(std::format("Loaded file: {}",ConvertString(filePath)));
#endif // _DEBUG
		return iDxcBlobMap_.at(filePath);
	}

	// 1:ファイル読み込み
	// これからシェーダーをコンパイルする旨をログに出ぁE
	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなぁE��ら停止
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの冁E��を設定すめE
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2:コンパイルする
	LPCWSTR arguments[] = {
		filePath.c_str(),		// コンパイル対象のhlslファイル吁E
		L"-E",L"main",			// エントリーポイント�E持E��。基本main以外にしなぁE
		L"-T",profile,			// ShaderProfileの設宁E
		L"-Zi",L"-Qembed_debug",// チE��チE��用の惁E��を埋め込む
		L"-Od",					// 最適化を外しておく
		L"-Zpr",				// レイアウト�E行優允E
	};
	// 実際にコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,		// 読み込んだファイル
		arguments,					// コンパイルオプション
		_countof(arguments),		// コンパイルオプション数
		includeHandler_,				// includeが含まれた諸、E
		IID_PPV_ARGS(&shaderResult)	// コンパイル結果
	);
	assert(SUCCEEDED(hr));

	// 3:エラー確誁E
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	// 4:コンパイル結果を受け取って返す
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"Compile Succeded, path:{},profile:{}\n", filePath, profile)));
	shaderSource->Release();
	shaderResult->Release();

	// 5:シェーダーリフレクション惁E��を外部に出力すめE
	ShaderReflection shaderReflection;
	shaderReflection.RunShaderReflection(shaderBlob);
	nlohmann::json shaderJson = shaderReflection.Serialize();
	std::string savePath = "Resources/TestFolder/" + ConvertString(filePath.substr(filePath.find_last_of(L"/\\") + 1)) + ".json";
	QFE::FILE::SaveJSONToFile( savePath, shaderJson);

	// シェーダーを登録
	iDxcBlobMap_.emplace(filePath, shaderBlob);
	return shaderBlob;
}
