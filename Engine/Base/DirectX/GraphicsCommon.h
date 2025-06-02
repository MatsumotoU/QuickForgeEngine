#pragma once
#include "PipelineStateObject.h"
#include "DepthStencil.h"

class GraphicsCommon
{
public:
	void Initialize(DirectXCommon* dxCommon, WinApp* winApp);

public:
	PipelineStateObject* GetTrianglePso(BlendMode blendmode) { return &trianglePso_[static_cast<uint32_t>(blendmode)]; }
	PipelineStateObject* GetLinePso(BlendMode blendmode) { return &linePso_[static_cast<uint32_t>(blendmode)]; }
	PipelineStateObject* GetPointPso(BlendMode blendmode) { return &pointPso_[static_cast<uint32_t>(blendmode)]; }

	PipelineStateObject* GetParticlePso(BlendMode blendmode) { return &particlePso_[static_cast<uint32_t>(blendmode)]; }

	PipelineStateObject* GetGrayScalePso() { return &grayScaleTrianglePso_; }

	DepthStencil* GetDepthStencil() { return &depthStencil_; }

private: // メンバ変数
	DepthStencil depthStencil_;

	// PSO
	PipelineStateObject trianglePso_[kCountOfBlendMode];
	PipelineStateObject linePso_[kCountOfBlendMode];
	PipelineStateObject pointPso_[kCountOfBlendMode];

	PipelineStateObject particlePso_[kCountOfBlendMode];

	PipelineStateObject grayScaleTrianglePso_;
};