#include "RaytracingAccelerationStructure.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

namespace {
	// --- 【追加】テスト用の三角形の頂点データ（Positionのみ） ---
	struct Vertex { float x, y, z; };
	Vertex testVertices[] = {
		{  0.0f,  0.5f, 0.0f }, // 上
		{  0.5f, -0.5f, 0.0f }, // 右下
		{ -0.5f, -0.5f, 0.0f }  // 左下
	};
	const UINT vertexBufferSize = sizeof(testVertices);


}

bool RaytracingAccelerationStructure::CreateTestBLAS(ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4) {
	// テスト用の頂点バッファをその場（アップロードヒープ）で作る
	D3D12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = vertexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = device5->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&testVertexBuffer)
	);
	if (FAILED(hr)) return false;
	testVertexBuffer->SetName(L"Test-Vertex-Buffer");

	// データをGPUメモリにコピー
	void* pVertexDataBegin = nullptr;
	testVertexBuffer->Map(0, nullptr, &pVertexDataBegin);
	memcpy(pVertexDataBegin, testVertices, vertexBufferSize);
	testVertexBuffer->Unmap(0, nullptr);

	// --- ここで、作成したバッファの「本当のGPU住所」を取得する！ ---
	D3D12_GPU_VIRTUAL_ADDRESS testVertexBufferAddress = testVertexBuffer->GetGPUVirtualAddress();

	// DXRに「このポリゴン群をツリー化してね」と伝えるための構造体
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES; // 三角形ポリゴン
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;   // 不透明ポリゴンとして最適化

	// 三角形の詳細設定
	auto& triangles = geometryDesc.Triangles;
	triangles.VertexBuffer.StartAddress = testVertexBufferAddress;
	triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3; // float3 (Position)
	triangles.VertexCount = 3;                                // 3頂点（三角形1つ）
	triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;     // float3
	triangles.IndexCount = 0;                                 // インデックスなし（順番に描く）
	triangles.Transform3x4 = 0;                               // 変形行列はなし

	// --- ステップ2: 構築に必要なサイズをDX12に計算してもらう ---
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS buildInputs{};
	buildInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL; // BLASを指定
	buildInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE; // 描画最速モード
	buildInputs.NumDescs = 1;
	buildInputs.pGeometryDescs = &geometryDesc;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
	// DX12デバイスにサイズを計算してもらう
	device5->GetRaytracingAccelerationStructurePrebuildInfo(&buildInputs, &prebuildInfo);

	QFE_LOG(std::format("BLAS Result Size: {} bytes, Scratch Size: {} bytes",
		prebuildInfo.ResultDataMaxSizeInBytes, prebuildInfo.ScratchDataSizeInBytes));

	// --- ステップ3: 計算されたサイズを元に、GPUバッファ（UAVフラグ必須）を生成する ---
	// ※注意: 加速構造のバッファは D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS (UAV) と
	//       D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE 状態である必要があります。

	D3D12_HEAP_PROPERTIES defaultHeapProps{ D3D12_HEAP_TYPE_DEFAULT };

	// 実体バッファの作成
	D3D12_RESOURCE_DESC resultDesc = {};
	resultDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resultDesc.Width = prebuildInfo.ResultDataMaxSizeInBytes;
	resultDesc.Height = 1;
	resultDesc.DepthOrArraySize = 1;
	resultDesc.MipLevels = 1;
	resultDesc.Format = DXGI_FORMAT_UNKNOWN;
	resultDesc.SampleDesc.Count = 1;
	resultDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resultDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // UAV必須

	device5->CreateCommittedResource(
		&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resultDesc,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, // 専用の状態
		nullptr, IID_PPV_ARGS(&blasResultBuffer_)
	);
	blasResultBuffer_->SetName(L"BLAS-Result-Buffer");

	// スクラッチ（作業用）バッファの作成
	D3D12_RESOURCE_DESC scratchDesc = resultDesc;
	scratchDesc.Width = prebuildInfo.ScratchDataSizeInBytes;

	device5->CreateCommittedResource(
		&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &scratchDesc,
		D3D12_RESOURCE_STATE_COMMON, // 通常の状態
		nullptr, IID_PPV_ARGS(&blasScratchBuffer_)
	);
	blasScratchBuffer_->SetName(L"BLAS-Scratch-Buffer");

	// --- ステップ4: コマンドリストに構築コマンドを積む ---
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
	buildDesc.Inputs = buildInputs;
	buildDesc.DestAccelerationStructureData = blasResultBuffer_->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = blasScratchBuffer_->GetGPUVirtualAddress();

	// GPUに「この三角形をツリー構造に変換せよ」と命令する
	commandList4->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// 構築が終わるまで次の処理が走らないようにUAVバリア（同期）を貼る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = blasResultBuffer_.Get();
	commandList4->ResourceBarrier(1, &barrier);

	QFE_LOG("BLAS Build command recorded successfully.");
	return true;
}

bool QFE::GRAPHIC::RaytracingAccelerationStructure::CreateTestTLAS(ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4) {
	// 1. インスタンス（配置図）の記述を設定
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc{};

	// 3x4の変形行列（今回は位置は原点、回転なし、拡大率1の単位行列）
	instanceDesc.Transform[0][0] = 1.0f;
	instanceDesc.Transform[1][1] = 1.0f;
	instanceDesc.Transform[2][2] = 1.0f;

	instanceDesc.InstanceID = 0;       // シェーダー（定数バッファ等）側で「InstanceID()」として取得できる識別番号
	instanceDesc.InstanceMask = 0xFF;   // すべてのレイ（光線）を等しくヒットさせるマスク設定
	instanceDesc.InstanceContributionToHitGroupIndex = 0; // シェーダーテーブルのオフセット（今は0でOK）
	instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;


	// ★最重要: 前段で作成した BLAS のGPUアドレスをここで紐付ける
	if (!blasResultBuffer_) return false;
	instanceDesc.AccelerationStructure = blasResultBuffer_->GetGPUVirtualAddress();

	// -------------------------------------------------------------------------
	// 2. インスタンス記述を載せるための GPU バッファ (Upload Heap) を生成してデータをコピー
	// -------------------------------------------------------------------------
	const UINT instanceBufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC);

	D3D12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = instanceBufferSize; // 構造体1つ分のサイズ
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// インスタンスバッファの実体生成（メンバ変数 instanceDescBuffer_ に保持）
	HRESULT hr = device5->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instanceDescBuffer_)
	);
	if (FAILED(hr)) {
		QFE_LOG("Failed to create Instance Desc Buffer for TLAS.");
		return false;
	}
	instanceDescBuffer_->SetName(L"TLAS-Instance-Desc-Buffer");

	// CPU上のデータをGPUメモリ（Upload Heap）へ転送
	void* pInstanceDataBegin = nullptr;
	instanceDescBuffer_->Map(0, nullptr, &pInstanceDataBegin);
	std::memcpy(pInstanceDataBegin, &instanceDesc, instanceBufferSize);
	instanceDescBuffer_->Unmap(0, nullptr);

	

	// -------------------------------------------------------------------------
	// 3. TLAS ビルドに必要なサイズ（Result / Scratch）を計算してもらう
	// -------------------------------------------------------------------------
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS buildInputs{};
	buildInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL; // ★今回はTOP_LEVELを指定
	buildInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	buildInputs.NumDescs = 1; // 配置するインスタンスの数（今回は三角形1つ分なので1）
	buildInputs.InstanceDescs = instanceDescBuffer_->GetGPUVirtualAddress();   // 先ほど転送したバッファのGPUアドレス

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
	device5->GetRaytracingAccelerationStructurePrebuildInfo(&buildInputs, &prebuildInfo);

	QFE_LOG(std::format("TLAS Result Size: {} bytes, Scratch Size: {} bytes",prebuildInfo.ResultDataMaxSizeInBytes, prebuildInfo.ScratchDataSizeInBytes));

	// -------------------------------------------------------------------------
	// 4. 計算されたサイズを元に、TLAS用バッファ（実体用・作業用）を生成
	// -------------------------------------------------------------------------
	D3D12_HEAP_PROPERTIES defaultHeapProps{ D3D12_HEAP_TYPE_DEFAULT };

	// TLAS実体バッファの生成設定
	D3D12_RESOURCE_DESC tlasDesc = bufferDesc;
	tlasDesc.Width = prebuildInfo.ResultDataMaxSizeInBytes;
	tlasDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // UAVフラグ必須

	hr = device5->CreateCommittedResource(
		&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &tlasDesc,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, // 加速構造専用の状態
		nullptr, IID_PPV_ARGS(&tlasResultBuffer_)
	);
	if (FAILED(hr)) return false;
	tlasResultBuffer_->SetName(L"TLAS-Result-Buffer");

	// TLAS作業用（スクラッチ）バッファの生成設定
	D3D12_RESOURCE_DESC scratchDesc = bufferDesc;
	scratchDesc.Width = prebuildInfo.ScratchDataSizeInBytes;
	scratchDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // UAVフラグ必須

	hr = device5->CreateCommittedResource(
		&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &scratchDesc,
		D3D12_RESOURCE_STATE_COMMON, // 通常の状態
		nullptr, IID_PPV_ARGS(&tlasScratchBuffer_)
	);
	if (FAILED(hr)) return false;
	tlasScratchBuffer_->SetName(L"TLAS-Scratch-Buffer");

	// -------------------------------------------------------------------------
	// 5. コマンドリストに構築コマンドを記録
	// -------------------------------------------------------------------------
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
	buildDesc.Inputs = buildInputs;
	buildDesc.DestAccelerationStructureData = tlasResultBuffer_->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = tlasScratchBuffer_->GetGPUVirtualAddress();

	// GPUに「シーンの配置図（TLAS）を組み立てよ」と命令
	commandList4->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// 構築完了を同期するためのUAVバリアを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = tlasResultBuffer_.Get();
	commandList4->ResourceBarrier(1, &barrier);

	QFE_LOG("TLAS Build command recorded successfully.");
	return true;
}

