#include "BLAS.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

bool QFE::GRAPHIC::BLAS::Create(ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4, std::vector<QFE::MATH::Vector3> vertices) {
	// 頂点バッファを作る
	if (!CreateVertexPositionBuffer(device5, vertices)) {
		QFE_LOG("Failed to create vertex position buffer for BLAS.");
		return false;
	}
	// BLASリソースを作る
	if (!CreateBLASResource(device5, commandList4, vertices)) {
		QFE_LOG("Failed to create BLAS resource.");
		return false;
	}
	isCreated_ = true;
	return true;
}

bool QFE::GRAPHIC::BLAS::CreateVertexPositionBuffer(
	ID3D12Device5* device5, std::vector<QFE::MATH::Vector3> vertices) {
	// すでに作成済みなら作らない
	if(isCreated_) {
		QFE_LOG("BLAS is already created. Cannot create vertex position buffer again.");
		return false;
	}

	// 頂点バッファのサイズを計算
	vertexBufferSize_ = sizeof(QFE::MATH::Vector3) * vertices.size();
	vertexCount_ = static_cast<UINT>(vertices.size());

	// テスト用の頂点バッファをその場（アップロードヒープ）で作る
	D3D12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = vertexBufferSize_;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = device5->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexPositionBuffer_)
	);
	if (FAILED(hr)) return false;
	vertexPositionBuffer_->SetName(L"Vertex-Position-Buffer");

	// データをGPUメモリにコピー
	void* pVertexDataBegin = nullptr;
	vertexPositionBuffer_->Map(0, nullptr, &pVertexDataBegin);
	memcpy(pVertexDataBegin, vertices.data(), vertexBufferSize_);
	vertexPositionBuffer_->Unmap(0, nullptr);

	return true;
}

bool BLAS::CreateBLASResource(
	ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4, std::vector<QFE::MATH::Vector3> vertices) {

	// すでに作成済みなら作らない
	if(isCreated_) {
		QFE_LOG("BLAS is already created. Cannot create BLAS resource again.");
		return false;
	}

	// DXRに「このポリゴン群をツリー化してね」と伝えるための構造体
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES; // 三角形ポリゴン
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;   // 不透明ポリゴンとして最適化

	// ポリゴンの詳細設定
	auto& triangles = geometryDesc.Triangles;
	triangles.VertexBuffer.StartAddress = vertexPositionBuffer_->GetGPUVirtualAddress();
	triangles.VertexBuffer.StrideInBytes = sizeof(QFE::MATH::Vector3); // Vector3 (Position)
	triangles.VertexCount = vertexCount_;                              // 頂点の数
	triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;     // float3
	triangles.IndexCount = 0;                                 // インデックスなし（順番に描く）
	triangles.Transform3x4 = 0;								// 変換行列なし（ワールド座標系のまま）                   

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
