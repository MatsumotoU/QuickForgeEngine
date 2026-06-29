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

bool RaytracingPSO::CreateShaderTables(ID3D12Device5* device) {
	if (!raytracingPipelineState_) return false;

	// 1. RTPSO から「ID3D12StateObjectProperties」インターフェースを引き出す
	// これを介して、各関数の識別子（Shader Identifier）を取得します。
	Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> rtpsoProps;
	HRESULT hr = raytracingPipelineState_->QueryInterface(IID_PPV_ARGS(&rtpsoProps));
	if (FAILED(hr)) return false;

	// 2. HLSLで定義した関数名を使って、それぞれの識別子（32バイトのポインタ）を取得
	// ★マングリング名ではなく、HLSL側のプレーンな名前で取得できます
	void* rayGenId = rtpsoProps->GetShaderIdentifier(L"MyRayGen");
	void* missId = rtpsoProps->GetShaderIdentifier(L"MyMiss");

	if (!rayGenId || !missId) {
		QFE_LOG("Failed to get Shader Identifiers. Check your shader function names.");
		return false;
	}

	// 3. サイズの計算（32バイトの実データを、64バイトアライメントに切り上げる）
	const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; // 32
	const UINT shaderRecordSize = (shaderIdSize + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // これで 64 バイトになる

	// -------------------------------------------------------------------------
	// 4. RayGeneration テーブル（バッファ）の生成と書き込み
	// -------------------------------------------------------------------------
	D3D12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = shaderRecordSize; // 64バイト確保
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// RayGenバッファ作成
	hr = device->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&rayGenShaderTable_)
	);
	if (FAILED(hr)) return false;

	// 識別子（バイナリデータ）をバッファに書き込む
	void* pRayGenData = nullptr;
	rayGenShaderTable_->Map(0, nullptr, &pRayGenData);
	std::memcpy(pRayGenData, rayGenId, shaderIdSize); // 32バイト分コピー（残り32バイトはパディングとして放置）
	rayGenShaderTable_->Unmap(0, nullptr);

	// -------------------------------------------------------------------------
	// 5. Miss テーブル（バッファ）の生成と書き込み
	// -------------------------------------------------------------------------
	// 将来的に複数のMissシェーダーを持つ可能性を考慮して、今回は1つ分（64バイト）で作成
	bufferDesc.Width = shaderRecordSize;

	hr = device->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&missShaderTable_)
	);
	if (FAILED(hr)) return false;

	// 識別子をバッファに書き込む
	void* pMissData = nullptr;
	missShaderTable_->Map(0, nullptr, &pMissData);
	std::memcpy(pMissData, missId, shaderIdSize);
	missShaderTable_->Unmap(0, nullptr);

	QFE_LOG("RayGeneration and Miss Shader Tables created successfully.");
	return true;
}
