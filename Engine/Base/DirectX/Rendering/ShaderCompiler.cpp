#include "ShaderCompiler.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#include <cassert>

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG


ShaderCompiler::ShaderCompiler() {
	iDxcBlobMap_.clear();
}

ShaderCompiler::~ShaderCompiler() {
#ifdef _DEBUG
	DebugLog("=====ShaderFiles=====");
#endif // _DEBUG
	
	// iDxcBlobMap_に格納されているIDxcBlob*をすべてReleaseしてからクリア
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

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	includeHandler_ = nullptr;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// 既に読み込み済みのシェーダーを再度読み込まない
	if (iDxcBlobMap_.contains(filePath)) {
#ifdef _DEBUG
		DebugLog(std::format("Loaded file: {}",ConvertString(filePath)));
#endif // _DEBUG
		return iDxcBlobMap_.at(filePath);
	}

	// 1:ファイル読み込み
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めないなら停止
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2:コンパイルする
	LPCWSTR arguments[] = {
		filePath.c_str(),		// コンパイル対象のhlslファイル名
		L"-E",L"main",			// エントリーポイントの指定。基本main以外にしない
		L"-T",profile,			// ShaderProfileの設定
		L"-Zi",L"-Qembed_debug",// デバッグ用の情報を埋め込む
		L"-Od",					// 最適化を外しておく
		L"-Zpr",				// レイアウトは行優先
	};
	// 実際にコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,		// 読み込んだファイル
		arguments,					// コンパイルオプション
		_countof(arguments),		// コンパイルオプション数
		includeHandler_,				// includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)	// コンパイル結果
	);
	assert(SUCCEEDED(hr));

	// 3:エラー確認
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

	// シェーダーを登録
	iDxcBlobMap_.emplace(filePath, shaderBlob);
	return shaderBlob;
}

bool ShaderCompiler::ReflectShader(IDxcBlob* shaderBlob, ShaderReflection& reflection) {
	if (!shaderBlob) return false;
	return reflection.Reflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
}
