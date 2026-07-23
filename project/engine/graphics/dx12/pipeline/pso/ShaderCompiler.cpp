#include "ShaderCompiler.h"
#include "ShaderReflection.h"

#include <cassert>
#include <format>

#include "EngineDefines.h"
#include "string/MyString.h"
#include "file/FileUtility.h"

using namespace QFE::GRAPHIC;

void ShaderCompiler::Initialize() {
	dxcBlobMap_.clear();
	dxcDevice_.Initialize();
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// すでにコンパイル済みならキャッシュから取得
	if (dxcBlobMap_.contains(filePath)) {
		QFE_LOG(std::format("Loaded file: {}", ConvertString(filePath)));
		return dxcBlobMap_.at(filePath).Get();
	}

	// 絶対パスを取得
	std::wstring absPath = QFE::FILE::GetAbsolutePath(filePath);
	QFE_LOG(ConvertString(std::format(L"Shader file absolute path: {}\n", absPath)));

	// hlslファイルをロード
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource;
	HRESULT hr = dxcDevice_.GetDxcUtils()->LoadFile(filePath.c_str(), nullptr, shaderSource.GetAddressOf());

	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("ShaderCompiler: Failed to load shader file: {}", ConvertString(filePath)), SystemError::Abort);
		return nullptr;
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
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult;
	hr = dxcDevice_.GetDxcCompiler()->Compile(
		&shaderSourceBuffer,		// 入力ファイル
		arguments,					// 引数
		_countof(arguments),		// 引数数
		dxcDevice_.GetIncludeHandler(),			// インクルードハンドラ
		IID_PPV_ARGS(shaderResult.GetAddressOf())	// 結果
	);
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("ShaderCompiler: DXC invocation failed.", SystemError::Abort);
		return nullptr;
	}

	// エラー出力を取得
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError;
	hr = shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(shaderError.GetAddressOf()), nullptr);
	if (SUCCEEDED(hr) && shaderError != nullptr && shaderError->GetStringLength() != 0) {
		QFE_LOG(std::format(
			"Shader compiler diagnostics for file: {}:\n{}",
			ConvertString(filePath), shaderError->GetStringPointer()));
	}

	// バイナリ出力を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(shaderBlob.GetAddressOf()), nullptr);
	if (FAILED(hr) || shaderBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::format("Shader compilation failed: {}", ConvertString(filePath)), SystemError::Abort);
		return nullptr;
	}
	QFE_LOG(ConvertString(std::format(L"Compile Succeeded, path:{},profile:{}\n", filePath, profile)));

	// キャッシュに保存
	dxcBlobMap_.emplace(filePath, shaderBlob);
	return shaderBlob.Get();
}

IDxcBlob* ShaderCompiler::ForceCompileShader(const std::wstring& filePath, const wchar_t* profile) {

	// 絶対パスを取得
	std::wstring absPath = QFE::FILE::GetAbsolutePath(filePath);
	QFE_LOG(ConvertString(std::format(L"Shader file absolute path: {}\n", absPath)));

	// hlslファイルをロード
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource;
	HRESULT hr = dxcDevice_.GetDxcUtils()->LoadFile(filePath.c_str(), nullptr, shaderSource.GetAddressOf());

	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("ShaderCompiler: Failed to load shader file: {}", ConvertString(filePath)), SystemError::Abort);
		return nullptr;
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
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult;
	hr = dxcDevice_.GetDxcCompiler()->Compile(
		&shaderSourceBuffer,		// 入力ファイル
		arguments,					// 引数
		_countof(arguments),		// 引数数
		dxcDevice_.GetIncludeHandler(),			// インクルードハンドラ
		IID_PPV_ARGS(shaderResult.GetAddressOf())	// 結果
	);
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("ShaderCompiler: DXC invocation failed.", SystemError::Abort);
		return nullptr;
	}

	// エラー出力を取得
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError;
	hr = shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(shaderError.GetAddressOf()), nullptr);
	if (SUCCEEDED(hr) && shaderError != nullptr && shaderError->GetStringLength() != 0) {
		QFE_LOG(std::format(
			"Shader compiler diagnostics for file: {}:\n{}",
			ConvertString(filePath), shaderError->GetStringPointer()));
	}

	// バイナリ出力を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(shaderBlob.GetAddressOf()), nullptr);
	if (FAILED(hr) || shaderBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::format("Shader compilation failed: {}", ConvertString(filePath)), SystemError::Abort);
		return nullptr;
	}
	QFE_LOG(ConvertString(std::format(L"Compile Succeeded, path:{},profile:{}\n", filePath, profile)));

	// キャッシュを更新
	if (dxcBlobMap_.contains(filePath)) {
		dxcBlobMap_[filePath] = shaderBlob;
		QFE_LOG(std::format("Recompiled file: {}", ConvertString(filePath)));
	}
	else {
		dxcBlobMap_.emplace(filePath, shaderBlob);
		QFE_LOG(std::format("Compiled file: {}", ConvertString(filePath)));
	}
	return shaderBlob.Get();
}

void ShaderCompiler::Finalize() {
	dxcDevice_.Finalize();

	QFE_LOG("=====ShaderFiles=====");

	for (const auto& [key, blob] : dxcBlobMap_) {
		QFE_LOG(std::format("Delete: {}", ConvertString(key)));
	}
	dxcBlobMap_.clear();

	QFE_LOG("=====================");
}
