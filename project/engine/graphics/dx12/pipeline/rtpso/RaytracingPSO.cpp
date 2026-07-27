#include "RaytracingPSO.h"
#include "RaytracingAccelerationStructure.h"
#include "../pso/StaticSamplerTemplate.h"

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
	D3D12_EXPORT_DESC exports[3] = {};
	exports[0].Name = L"MyRayGen";
	exports[0].ExportToRename = nullptr;
	exports[0].Flags = D3D12_EXPORT_FLAG_NONE;

	exports[1].Name = L"MyMiss";
	exports[1].ExportToRename = nullptr;
	exports[1].Flags = D3D12_EXPORT_FLAG_NONE;

	exports[2].Name = L"MyClosestHit";
	exports[2].ExportToRename = nullptr;
	exports[2].Flags = D3D12_EXPORT_FLAG_NONE;

	dxilLibraryDesc.NumExports = 3;
	dxilLibraryDesc.pExports = exports;

	// 3. ShaderConfigの設定
	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
	shaderConfig.MaxPayloadSizeInBytes = 32; // ペイロードの最大サイズ
	shaderConfig.MaxAttributeSizeInBytes = 8; // 属性の最大サイズ

	// 4. PipelineConfigの設定
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
	// RayGen -> reflection -> shadow。反射の追加バウンスは行わない。
	pipelineConfig.MaxTraceRecursionDepth = 2;

	// 5. サブオブジェクトの設定
	D3D12_STATE_SUBOBJECT subobjects[5] = {};
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

	// 5. Hit Groupの設定
	D3D12_HIT_GROUP_DESC hitGroupDesc = {};
	hitGroupDesc.HitGroupExport = L"MyHitGroup"; // C++側やShaderTableで参照するグループ名
	hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
	hitGroupDesc.ClosestHitShaderImport = L"MyClosestHit"; // 上でエクスポートした名前
	hitGroupDesc.AnyHitShaderImport = nullptr;
	hitGroupDesc.IntersectionShaderImport = nullptr;

	subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	subobjects[subobjectIndex].pDesc = &hitGroupDesc;
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

	Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> rtpsoProps;
	HRESULT hr = raytracingPipelineState_->QueryInterface(IID_PPV_ARGS(&rtpsoProps));
	if (FAILED(hr)) return false;

	// 1. 各識別子の取得（★MyHitGroup を追加）
	void* rayGenId = rtpsoProps->GetShaderIdentifier(L"MyRayGen");
	void* missId = rtpsoProps->GetShaderIdentifier(L"MyMiss");
	void* hitGroupId = rtpsoProps->GetShaderIdentifier(L"MyHitGroup"); // ⭕グループ名で取得します

	if (!rayGenId || !missId || !hitGroupId) {
		QFE_LOG("Failed to get Shader Identifiers.");
		return false;
	}

	// レコードサイズ（64バイト）と、安全なバッファ全体のサイズ（4096バイト）の計算
	const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; // 32
	const UINT shaderRecordSize = (shaderIdSize + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 64
	const UINT safeBufferSize = (shaderRecordSize + 4095) & ~4095; // 4096

	D3D12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = safeBufferSize;
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
	bufferDesc.Width = safeBufferSize;

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

	// -------------------------------------------------------------------------
	// 2. ★HitGroup テーブル（バッファ）の生成と書き込みを追加
	// -------------------------------------------------------------------------
	// メンバ変数、もしくは管理クラス内に hitGroupShaderTable_ (ComPtr<ID3D12Resource>) を用意してください
	hr = device->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&hitGroupShaderTable_)
	);
	if (FAILED(hr)) return false;

	// 識別子（バイナリデータ）をバッファに書き込む
	void* pHitGroupData = nullptr;
	hitGroupShaderTable_->Map(0, nullptr, &pHitGroupData);
	std::memcpy(pHitGroupData, hitGroupId, shaderIdSize); // 32バイトコピー
	hitGroupShaderTable_->Unmap(0, nullptr);

	QFE_LOG("RayGeneration, Miss, and HitGroup Shader Tables created successfully.");
	return true;
}
