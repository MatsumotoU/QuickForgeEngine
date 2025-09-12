#include "GraphicPipelineManager.h"

void GraphicPipelineManager::Initialize(
	ID3D12Device* device, uint32_t width, uint32_t height, DsvDescriptorHeap* dsvHeap) {
	depthStencil_.CreateDepthStencilTextureResource(device, width,height,dsvHeap);
	shaderCompiler_.InitializeDXC();

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
		trianglePso_[i].Initialize(&shaderCompiler_,device);
		trianglePso_[i].CreatePipelineStateObject(
			normalGameObjectRootParameter_, depthStencil_.GetDepthStencilDesc(), normalInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", "Object3d.VS.hlsl", static_cast<BlendMode>(i),false);
		
		linePso_[i].Initialize(&shaderCompiler_, device);
		linePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, depthStencil_.GetDepthStencilDesc(), primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);
		
		pointPso_[i].Initialize(&shaderCompiler_, device);
		pointPso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, depthStencil_.GetDepthStencilDesc(), primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);
		
		primitivePso_[i].Initialize(&shaderCompiler_, device);
		primitivePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, depthStencil_.GetDepthStencilDesc(), primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);
		
		particlePso_[i].Initialize(&shaderCompiler_, device);
		particlePso_[i].CreatePipelineStateObject(
			particleRootParameter_, depthStencil_.GetDepthStencilDesc(), normalInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Particle.PS.hlsl", "Particle.VS.hlsl", static_cast<BlendMode>(i),false);
	}
	
	colorCorrectionPso_.Initialize(&shaderCompiler_, device);
	colorCorrectionPso_.CreatePipelineStateObject(
		colorCorrectionRootParameter_, depthStencil_.GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "ColorCorrectionShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	grayScaleTrianglePso_.Initialize(&shaderCompiler_, device);
	grayScaleTrianglePso_.CreatePipelineStateObject(
		grayScaleRootParameter_, depthStencil_.GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "GrayscaleShader.hlsl","Simple.VS.hlsl", kBlendModeNormal, false);

	vignettePso_.Initialize(&shaderCompiler_, device);
	vignettePso_.CreatePipelineStateObject(
		vignetteRootParameter_, depthStencil_.GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "VignetteShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	normalPso_.Initialize(&shaderCompiler_, device);
	normalPso_.CreatePipelineStateObject(
		grayScaleRootParameter_, depthStencil_.GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Simple.PS.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	fontPso_.Initialize(&shaderCompiler_, device);
	fontPso_.CreatePipelineStateObject(
		fontRootParameter_, depthStencil_.GetDepthStencilDesc(), primitiveInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "MSDF.PS.hlsl", "Font.VS.hlsl", kBlendModeNormal, false);
}