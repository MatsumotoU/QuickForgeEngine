#pragma once
#include "PipelineStateObject.h"
#include "DepthStencil.h"

class GraphicsCommon
{
public:
	void Initialize(DirectXCommon* dxCommon, WinApp* winApp);

public:
	void ChangeFillMode(D3D12_FILL_MODE fillMode);

public:
	PipelineStateObject* GetTrianglePso() { return &trianglePso_; }
	PipelineStateObject* GetLinePso() { return &linePso_; }
	PipelineStateObject* GetPointPso() { return &pointPso_; }

private: // メンバ変数
	DepthStencil depthStencil_;

	// PSO
	PipelineStateObject trianglePso_;
	PipelineStateObject linePso_;
	PipelineStateObject pointPso_;
};