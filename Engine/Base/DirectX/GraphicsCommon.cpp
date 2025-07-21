#include "GraphicsCommon.h"
#include "Base/EngineCore.h"

void GraphicsCommon::Initialize(EngineCore* engineCore) {
	depthStencil_.Initialize(engineCore->GetWinApp(), engineCore->GetDirectXCommon());
	// 通常のルートパラメータ
	normalGameObjectRootParameter_.Initialize();
	normalGameObjectRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	normalGameObjectRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("LightParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	normalGameObjectRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// パーティクルのルートパラメータ
	particleRootParameter_.Initialize();
	particleRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	particleRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	particleRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	particleRootParameter_.SetDescriptorRange("VertexParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	particleRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// プリミティブのルートパラメータ
	primitiveRootParameter_.Initialize();
	primitiveRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	primitiveRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);

	// 色調補正のやつ
	colorCorrectionRootParameter_.Initialize();
	colorCorrectionRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	colorCorrectionRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	
	colorCorrectionRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// グレースケールのやつ
	grayScaleRootParameter_.Initialize();
	grayScaleRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	grayScaleRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	grayScaleRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ビネットのやつ
	vignetteRootParameter_.Initialize();
	vignetteRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	vignetteRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	vignetteRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// 何もしないやつ
	normalRootParameter_.Initialize();
	normalRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// フォントのルートパラメータ
	fontRootParameter_.Initialize();
	fontRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	fontRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	fontRootParameter_.CreateRootParameter("FontParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	fontRootParameter_.SetDescriptorRange("VertexParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	fontRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// パラメータの整合性チェック
#ifdef _DEBUG
	normalGameObjectRootParameter_.CheckIntegrityData();
	particleRootParameter_.CheckIntegrityData();
	primitiveRootParameter_.CheckIntegrityData();
	grayScaleRootParameter_.CheckIntegrityData();
	vignetteRootParameter_.CheckIntegrityData();
	normalRootParameter_.CheckIntegrityData();
	colorCorrectionRootParameter_.CheckIntegrityData();
	fontRootParameter_.CheckIntegrityData();
#endif // _DEBUG

	// インプットレイアウトの初期化
	InputLayout normalInputLayout;
	normalInputLayout.CreateNormalPresetInputLayout();

	InputLayout primitiveInputLayout;
	primitiveInputLayout.CreatePrimitivePresetInputLayout();

	// PSOを作成
	for (int i = 0; i < kCountOfBlendMode; i++) {
		// 初期化
		trianglePso_[i].Initialize(engineCore);
		linePso_[i].Initialize(engineCore);
		pointPso_[i].Initialize(engineCore);
		particlePso_[i].Initialize(engineCore);
		primitivePso_[i].Initialize(engineCore);

		trianglePso_[i].CreatePipelineStateObject(
			normalGameObjectRootParameter_, &depthStencil_, normalInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", "Object3d.VS.hlsl", static_cast<BlendMode>(i),false);

		linePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, &depthStencil_, primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);

		pointPso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, &depthStencil_, primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);

		primitivePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_,&depthStencil_, primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);

		particlePso_[i].CreatePipelineStateObject(
			particleRootParameter_, &depthStencil_, normalInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Particle.PS.hlsl", "Particle.VS.hlsl", static_cast<BlendMode>(i),false);
	}
	
	colorCorrectionPso_.Initialize(engineCore);
	colorCorrectionPso_.CreatePipelineStateObject(
		colorCorrectionRootParameter_, &depthStencil_, normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "ColorCorrectionShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	grayScaleTrianglePso_.Initialize(engineCore);
	grayScaleTrianglePso_.CreatePipelineStateObject(
		grayScaleRootParameter_, &depthStencil_, normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "GrayscaleShader.hlsl","Simple.VS.hlsl", kBlendModeNormal, false);

	vignettePso_.Initialize(engineCore);
	vignettePso_.CreatePipelineStateObject(
		vignetteRootParameter_, &depthStencil_, normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "VignetteShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	normalPso_.Initialize(engineCore);
	normalPso_.CreatePipelineStateObject(
		grayScaleRootParameter_, &depthStencil_, normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Simple.PS.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	fontPso_.Initialize(engineCore);
	fontPso_.CreatePipelineStateObject(
		fontRootParameter_, &depthStencil_, primitiveInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "MSDF.PS.hlsl", "Font.VS.hlsl", kBlendModeNormal, false);
}