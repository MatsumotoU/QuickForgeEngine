#include "RaytracingPipelineManager.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void RaytracingPipelineManager::Initialize(const RaytracingPipelineManagerInitializeInfo& initializeInfo) {
	// デバイスを変換して保持
	HRESULT hr = initializeInfo.device->QueryInterface(IID_PPV_ARGS(&device_));
	if(FAILED(hr)) {
		QFE_LOG("RaytracingPipelineManager: Failed to query ID3D12Device5 interface from ID3D12Device.");
		return;
	}

	compileFunc = initializeInfo.compileFunc;

	isActive_ = true;
}

void RaytracingPipelineManager::Finalize() {
	if (isActive_) {
		device_.Reset();
		isActive_ = false;
	}
}

void QFE::GRAPHIC::RaytracingPipelineManager::CompileRaytracingShader(const std::wstring& filePath, const wchar_t* profile) {
	// シェーダーのコンパイルを実行
	IDxcBlob* shaderBlob = nullptr;
	if (compileFunc) {
		 shaderBlob = compileFunc(filePath, profile);
	}

	// シェーダーのコンパイル結果を確認
	if(shaderBlob) {
		// シェーダーのコンパイルが成功した場合の処理
		QFE_LOG("RaytracingPipelineManager: Shader compiled successfully.");
		shaderBlob->Release(); // 使用後はリリース
	} else {
		// シェーダーのコンパイルが失敗した場合の処理
		QFE_LOG("RaytracingPipelineManager: Shader compilation failed.");
	}
}
