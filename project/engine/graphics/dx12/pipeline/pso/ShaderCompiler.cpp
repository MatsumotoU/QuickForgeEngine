#include "ShaderCompiler.h"
#include "ShaderReflection.h"

#include <cassert>
#include <format>

#include "EngineDefines.h"
#include "string/MyString.h"
#include "file/FileUtility.h"

using namespace QFE::GRAPHIC;

void ShaderCompiler::Initialize() {
	iDxcBlobMap_.clear();
	dxcDevice_.Initialize();
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// すでにコンパイル済みならキャッシュから取得
	if (iDxcBlobMap_.contains(filePath)) {
		QFE_LOG(std::format("Loaded file: {}", ConvertString(filePath)));
		return iDxcBlobMap_.at(filePath);
	}

	// 絶対パスを取得
	std::wstring absPath = QFE::FILE::GetAbsolutePath(filePath);
	QFE_LOG(ConvertString(std::format(L"Shader file absolute path: {}\n", absPath)));

	// hlslファイルをロード
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcDevice_.GetDxcUtils()->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("ShaderCompiler: Failed to load shader file: {}", ConvertString(filePath)), SystemError::Abort);
	}

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
	hr = dxcDevice_.GetDxcCompiler()->Compile(
		&shaderSourceBuffer,		// 入力ファイル
		arguments,					// 引数
		_countof(arguments),		// 引数数
		dxcDevice_.GetIncludeHandler(),			// インクルードハンドラ
		IID_PPV_ARGS(&shaderResult)	// 結果
	);
	assert(SUCCEEDED(hr));

	// エラー出力を取得
	IDxcBlobUtf8* shaderError = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (SUCCEEDED(hr) && shaderError != nullptr && shaderError->GetStringLength() != 0) {
		QFE_LOG(std::format(
			"Shader compilation failed for file: {}, errors:\n{}",
			ConvertString(filePath), shaderError->GetStringPointer()));
		assert(false);
	}

	// バイナリ出力を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	QFE_LOG(ConvertString(std::format(L"Compile Succeded, path:{},profile:{}\n", filePath, profile)));
	shaderSource->Release();
	shaderResult->Release();

	// キャッシュに保存
	iDxcBlobMap_.emplace(filePath, shaderBlob);
	return shaderBlob;
}

IDxcBlob* ShaderCompiler::ForceCompileShader(const std::wstring& filePath, const wchar_t* profile) {

	// 絶対パスを取得
	std::wstring absPath = QFE::FILE::GetAbsolutePath(filePath);
	QFE_LOG(ConvertString(std::format(L"Shader file absolute path: {}\n", absPath)));

	// hlslファイルをロード
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcDevice_.GetDxcUtils()->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("ShaderCompiler: Failed to load shader file: {}", ConvertString(filePath)), SystemError::Abort);
	}

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
	hr = dxcDevice_.GetDxcCompiler()->Compile(
		&shaderSourceBuffer,		// 入力ファイル
		arguments,					// 引数
		_countof(arguments),		// 引数数
		dxcDevice_.GetIncludeHandler(),			// インクルードハンドラ
		IID_PPV_ARGS(&shaderResult)	// 結果
	);
	assert(SUCCEEDED(hr));

	// エラー出力を取得
	IDxcBlobUtf8* shaderError = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (SUCCEEDED(hr) && shaderError != nullptr && shaderError->GetStringLength() != 0) {
		QFE_LOG(std::format(
			"Shader compilation failed for file: {}, errors:\n{}",
			ConvertString(filePath), shaderError->GetStringPointer()));
		assert(false);
	}

	// バイナリ出力を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	QFE_LOG(ConvertString(std::format(L"Compile Succeded, path:{},profile:{}\n", filePath, profile)));
	shaderSource->Release();
	shaderResult->Release();

	// キャッシュを更新
	if (iDxcBlobMap_.contains(filePath)) {
		iDxcBlobMap_.at(filePath)->Release();
		iDxcBlobMap_[filePath] = shaderBlob;
		QFE_LOG(std::format("Recompiled file: {}", ConvertString(filePath)));
	}
	else {
		iDxcBlobMap_.emplace(filePath, shaderBlob);
		QFE_LOG(std::format("Compiled file: {}", ConvertString(filePath)));
	}
	return shaderBlob;
}

void ShaderCompiler::Finalize() {
	dxcDevice_.Finalize();

	QFE_LOG("=====ShaderFiles=====");

	// iDxcBlobMap_に格納されているIDxcBlob*をReleaseして解放
	for (auto& [key, blob] : iDxcBlobMap_) {
		if (blob) {
			blob->Release();
			QFE_LOG(std::format("Delete: {}", ConvertString(key)));
		}
	}
	iDxcBlobMap_.clear();

	QFE_LOG("=====================");
}