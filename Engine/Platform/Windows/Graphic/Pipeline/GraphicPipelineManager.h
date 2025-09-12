#pragma once
#include "PSO/PipelineStateObject.h"
#include "Platform/Windows/Graphic/DirectXCommon/Descriptors/DsvDescriptorHeap.h"
#include "Platform/Windows/Graphic/ShaderBuffer/DepthStencil.h"

class GraphicPipelineManager final {
public:
	void Initialize(ID3D12Device* device,uint32_t width,uint32_t height, DsvDescriptorHeap* dsvHeap);
public:
	/// <summary>
	/// 一般的な三角形のPSOを取得します
	/// </summary>
	/// <param name="blendmode">ブレンドモード</param>
	/// <returns></returns>
	PipelineStateObject* GetTrianglePso(BlendMode blendmode) { return &trianglePso_[static_cast<uint32_t>(blendmode)]; }
	/// <summary>
	/// 一般的な三角形のPSOを取得します
	/// </summary>
	/// <param name="blendmode">ブレンドモード</param>
	/// <returns></returns>
	PipelineStateObject* GetLinePso(BlendMode blendmode) { return &linePso_[static_cast<uint32_t>(blendmode)]; }
	/// <summary>
	/// 一般的な三角形のPSOを取得します
	/// </summary>
	/// <param name="blendmode">ブレンドモード</param>
	/// <returns></returns>
	PipelineStateObject* GetPointPso(BlendMode blendmode) { return &pointPso_[static_cast<uint32_t>(blendmode)]; }
	PipelineStateObject* GetPrimitivePso(BlendMode blendmode) { return &primitivePso_[static_cast<uint32_t>(blendmode)]; }
	PipelineStateObject* GetParticlePso(BlendMode blendmode) { return &particlePso_[static_cast<uint32_t>(blendmode)]; }
	PipelineStateObject* GetColorCorrectionPso() { return &colorCorrectionPso_; }
	PipelineStateObject* GetGrayScalePso() { return &grayScaleTrianglePso_; }
	PipelineStateObject* GetVignettePso() { return &vignettePso_; }
	PipelineStateObject* GetNormalPso() { return &normalPso_; }
	PipelineStateObject* GetFontPso() { return &fontPso_; }
	DepthStencil* GetDepthStencil() { return &depthStencil_; }

private: // メンバ変数
	DepthStencil depthStencil_;
	ShaderCompiler shaderCompiler_;

	// ルートパラメータ;
	RootParameter normalGameObjectRootParameter_;
	RootParameter particleRootParameter_;
	RootParameter primitiveRootParameter_;
	RootParameter grayScaleRootParameter_;
	RootParameter colorCorrectionRootParameter_;
	RootParameter vignetteRootParameter_;
	RootParameter normalRootParameter_;
	RootParameter fontRootParameter_;

	// PSO
	PipelineStateObject trianglePso_[kCountOfBlendMode];
	PipelineStateObject linePso_[kCountOfBlendMode];
	PipelineStateObject pointPso_[kCountOfBlendMode];

	PipelineStateObject primitivePso_[kCountOfBlendMode];

	PipelineStateObject particlePso_[kCountOfBlendMode];

	PipelineStateObject colorCorrectionPso_;
	PipelineStateObject grayScaleTrianglePso_;
	PipelineStateObject vignettePso_;

	PipelineStateObject fontPso_;

	PipelineStateObject normalPso_;
};