#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"

void GraphicPipelineManager::Initialize(
	ID3D12Device* device) {
	shaderCompiler_.InitializeDXC();

	// 通常のルートパラメータ
	normalGameObjectRootParameter_.Initialize();
	normalGameObjectRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	normalGameObjectRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("LightParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	normalGameObjectRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// パ�EチE�E��E�クルのルートパラメータ
	particleRootParameter_.Initialize();
	particleRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	particleRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	particleRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	particleRootParameter_.SetDescriptorRange("VertexParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	particleRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// プリミティブ�Eルートパラメータ
	primitiveRootParameter_.Initialize();
	primitiveRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	primitiveRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);

	// 色調補正のめE�E��E�
	colorCorrectionRootParameter_.Initialize();
	colorCorrectionRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	colorCorrectionRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	
	colorCorrectionRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// グレースケールのめE�E��E�
	grayScaleRootParameter_.Initialize();
	grayScaleRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	grayScaleRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	grayScaleRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ビネチE�E��E�のめE�E��E�
	vignetteRootParameter_.Initialize();
	vignetteRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	vignetteRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	vignetteRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ドット化のめE�E��E�
	pixcelRootParameter_.Initialize();
	pixcelRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	pixcelRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	
	pixcelRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// 何もしなぁE�E��E�つ
	normalRootParameter_.Initialize();
	normalRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// フォント�Eルートパラメータ
	fontRootParameter_.Initialize();
	fontRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	fontRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	fontRootParameter_.CreateRootParameter("FontParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	fontRootParameter_.SetDescriptorRange("VertexParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	fontRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// パラメータの整合性チェチE�E��E�
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

	// インプットレイアウト�E初期匁E
	InputLayout normalInputLayout;
	normalInputLayout.CreateNormalPresetInputLayout();

	InputLayout primitiveInputLayout;
	primitiveInputLayout.CreatePrimitivePresetInputLayout();

	// PSOを作�E
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	for (int i = 0; i < kCountOfBlendMode; i++) {
		trianglePso_[i].Initialize(&shaderCompiler_,device);
		trianglePso_[i].CreatePipelineStateObject(
			normalGameObjectRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", "Object3d.VS.hlsl", static_cast<BlendMode>(i),false);
		
		linePso_[i].Initialize(&shaderCompiler_, device);
		linePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);
		
		pointPso_[i].Initialize(&shaderCompiler_, device);
		pointPso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);
		
		primitivePso_[i].Initialize(&shaderCompiler_, device);
		primitivePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i),true);
		
		particlePso_[i].Initialize(&shaderCompiler_, device);
		particlePso_[i].CreatePipelineStateObject(
			particleRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Particle.PS.hlsl", "Particle.VS.hlsl", static_cast<BlendMode>(i),false);
	}
	
	colorCorrectionPso_.Initialize(&shaderCompiler_, device);
	colorCorrectionPso_.CreatePipelineStateObject(
		colorCorrectionRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "ColorCorrectionShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	grayScaleTrianglePso_.Initialize(&shaderCompiler_, device);
	grayScaleTrianglePso_.CreatePipelineStateObject(
		grayScaleRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "GrayscaleShader.hlsl","Simple.VS.hlsl", kBlendModeNormal, false);

	vignettePso_.Initialize(&shaderCompiler_, device);
	vignettePso_.CreatePipelineStateObject(
		vignetteRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "VignetteShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	normalPso_.Initialize(&shaderCompiler_, device);
	normalPso_.CreatePipelineStateObject(
		grayScaleRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Simple.PS.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	pixcelPso_.Initialize(&shaderCompiler_, device);
	pixcelPso_.CreatePipelineStateObject(
		pixcelRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "PixcelShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	fontPso_.Initialize(&shaderCompiler_, device);
	fontPso_.CreatePipelineStateObject(
		fontRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "MSDF.PS.hlsl", "Font.VS.hlsl", kBlendModeNormal, false);
}
