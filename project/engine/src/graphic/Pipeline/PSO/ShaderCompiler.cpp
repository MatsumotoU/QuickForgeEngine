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

	// iDxcBlobMap_に格納されているIDxcBlob*をReleaseして解放
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
	//// * DXCの初期化 * //
	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	// インクルード対応のためのハンドラ作成
	includeHandler_ = nullptr;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// すでにコンパイル済みならキャッシュから取得
	if (iDxcBlobMap_.contains(filePath)) {
#ifdef _DEBUG
		DebugLog(std::format("Loaded file: {}", ConvertString(filePath)));
#endif // _DEBUG
		return iDxcBlobMap_.at(filePath);
	}

	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));

	// 絶対パスを出力
	std::wstring absPath = QFE::FILE::GetAbsolutePath(filePath);
	Log(ConvertString(std::format(L"Shader file absolute path: {}\n", absPath)));

	// hlslファイルをロード
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));
	// バッファ作成
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// コンパイル引数
	LPCWSTR arguments[] = {
		filePath.c_str(),		// 入力hlslファイル
		L"-E",L"main",			// エントリーポイントmain
		L"-T",profile,			// ShaderProfile
		L"-Zi",L"-Qembed_debug",// デバッグ情報埋め込み
		L"-Od",					// 最適化無効
		L"-Zpr",				// 行優先パッキング
	};
	// シェーダーをコンパイル
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,		// 入力ファイル
		arguments,					// 引数
		_countof(arguments),		// 引数数
		includeHandler_,			// インクルードハンドラ
		IID_PPV_ARGS(&shaderResult)	// 結果
	);
	assert(SUCCEEDED(hr));

	// エラー出力を取得
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	// バイナリ出力を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"Compile Succeded, path:{},profile:{}\n", filePath, profile)));
	shaderSource->Release();
	shaderResult->Release();

	// シェーダーリフレクションを実行しJSON保存
	ShaderReflection shaderReflection;
	shaderReflection.RunShaderReflection(shaderBlob);
	nlohmann::json shaderJson = shaderReflection.Serialize();
	std::string savePath = "Resources/TestFolder/" + ConvertString(filePath.substr(filePath.find_last_of(L"/\\") + 1)) + ".json";
	QFE::FILE::SaveJSONToFile(savePath, shaderJson);

	// キャッシュに保存
	iDxcBlobMap_.emplace(filePath, shaderBlob);
	return shaderBlob;
}
