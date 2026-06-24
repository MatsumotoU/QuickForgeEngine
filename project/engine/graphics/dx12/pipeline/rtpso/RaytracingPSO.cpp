#include "RaytracingPSO.h"

using namespace QFE::GRAPHIC;

void RaytracingPSO::CreatePipelineStateObject(IDxcBlob* csBlob, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, ID3D12Device5* device) {
	// ルートシグネチャの作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			QFE_LOG("Failed to serialize root signature: " + std::string(static_cast<const char*>(errorBlob->GetBufferPointer())));
		}
		return;
	}
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr)) {
		QFE_LOG("Failed to create root signature.");
		return;
	}
	rootSignature_ = rootSignature;

	// レイトレーシングパイプラインステートオブジェクトの作成
	// 1. DXILライブラリの設定
	D3D12_DXIL_LIBRARY_DESC dxilLibraryDesc = {};
	dxilLibraryDesc.DXILLibrary.pShaderBytecode = csBlob->GetBufferPointer();// レイトレーシングシェーダーのバイナリデータ
	dxilLibraryDesc.DXILLibrary.BytecodeLength = csBlob->GetBufferSize();// バイナリデータのサイズ

	// 2. エクスポートの設定
	D3D12_EXPORT_DESC exports[2] = {};
	exports[0].Name = L"MyRayGen";
	exports[0].ExportToRename = nullptr;
	exports[0].Flags = D3D12_EXPORT_FLAG_NONE;

	exports[1].Name = L"MyMiss";
	exports[1].ExportToRename = nullptr;
	exports[1].Flags = D3D12_EXPORT_FLAG_NONE;

	dxilLibraryDesc.NumExports = 2;
	dxilLibraryDesc.pExports = exports;

	// 3. ShaderConfigの設定
	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
	shaderConfig.MaxPayloadSizeInBytes = 16; // ペイロードの最大サイズ
	shaderConfig.MaxAttributeSizeInBytes = 8; // 属性の最大サイズ

	// 4. PipelineConfigの設定
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
	pipelineConfig.MaxTraceRecursionDepth = 1; // 今回は反射させないので 1

	// 5. サブオブジェクトの設定
	D3D12_STATE_SUBOBJECT subobjects[4] = {};
	UINT subobjectIndex = 0;

	// 1. DXIL Library
	subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	subobjects[subobjectIndex].pDesc = &dxilLibraryDesc;
	subobjectIndex++;

	// 2. Global Root Signature
	subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	ID3D12RootSignature* pRawRootSig = rootSignature_.Get();
	subobjects[subobjectIndex].pDesc = &pRawRootSig;
	subobjectIndex++;
	// 3. Shader Config
	subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	subobjects[subobjectIndex].pDesc = &shaderConfig;
	subobjectIndex++;
	// 4. Pipeline Config
	subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	subobjects[subobjectIndex].pDesc = &pipelineConfig;
	subobjectIndex++;

	// 全体のパイプライン設定構造体にまとめる
	D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
	stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	stateObjectDesc.NumSubobjects = subobjectIndex;
	stateObjectDesc.pSubobjects = subobjects;

	// RTPSO（StateObject）を生成
	hr = device->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&raytracingPipelineState_));
	if (FAILED(hr)) {
		QFE_LOG("Failed to create Raytracing State Object.");
		return;
	}
}
