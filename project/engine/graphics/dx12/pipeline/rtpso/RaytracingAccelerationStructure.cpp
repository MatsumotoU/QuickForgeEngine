#include "RaytracingAccelerationStructure.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

bool RaytracingAccelerationStructure::CreateTestBLAS(ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4) {
	// --- ステップ1: テスト用の頂点データ（仮に前段で作成した頂点バッファのアドレスがあるとします） ---
	// 本来は引数などでVertex/IndexBufferのGPUアドレス（GetGPUVirtualAddress）を受け取ります
	D3D12_GPU_VIRTUAL_ADDRESS testVertexBufferAddress = 0; // ★本来はここに頂点バッファのアドレスが入る

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

	// スクラッチ（作業用）バッファの作成
	D3D12_RESOURCE_DESC scratchDesc = resultDesc;
	scratchDesc.Width = prebuildInfo.ScratchDataSizeInBytes;

	device5->CreateCommittedResource(
		&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &scratchDesc,
		D3D12_RESOURCE_STATE_COMMON, // 通常の状態
		nullptr, IID_PPV_ARGS(&blasScratchBuffer_)
	);

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
