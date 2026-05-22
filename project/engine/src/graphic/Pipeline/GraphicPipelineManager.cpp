/**
 * @file GraphicPipelineManager.cpp
 * @brief グラフィックスパイプラインおよびルートシグネチャの管理クラスの実装
 */
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
#include "engine/include/utility/String/MyString.h"

using namespace QFE;
 /**
  * @brief 各種パイプラインの初期化
  * @param device ID3D12Deviceへのポインタ
  */
namespace {
	const std::string kVSFilePath = "engine/resources/shaders/vs/";
	const std::string kPSFilePath = "engine/resources/shaders/ps/";
}

void GraphicPipelineManager::Initialize(
	ID3D12Device* device) {
	shaderCompiler_.InitializeDXC();
	// シェーダーを格納しているディレクトリ内のファイル名一覧を取得
	std::vector<std::string> vsFiles = QFE::FILE::GetFilesInDirectory(kVSFilePath);
	std::vector<std::string> psFiles = QFE::FILE::GetFilesInDirectory("engine/resources/shaders/ps");
	// シェーダーリフレクション情報を取得
	for (const auto& vsFile : vsFiles) {
		shaderCompiler_.CompileShader(ConvertString(kVSFilePath + vsFile), L"vs_6_0");
	}
	for (const auto& psFile : psFiles) {
		shaderCompiler_.CompileShader(ConvertString(kPSFilePath + psFile), L"ps_6_0");
	}

	// 全シェーダーのリフレクション情報をJSON形式で取得
	std::map<std::string, nlohmann::json> allShaderReflectionJson = shaderCompiler_.GetAllShaderReflectionJson();

	// 通常のルートパラメータ
	normalGameObjectRootParameter_.Initialize();
	normalGameObjectRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	normalGameObjectRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("CubeTextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 1);
	normalGameObjectRootParameter_.CreateRootParameter("LightParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 1);
	normalGameObjectRootParameter_.CreateRootParameter("CameraParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 2);

	normalGameObjectRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	normalGameObjectRootParameter_.SetDescriptorRange("CubeTextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	// スプライトのルートパラメータ
	spriteObjectRootParameter_.Initialize();
	spriteObjectRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	spriteObjectRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	spriteObjectRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	spriteObjectRootParameter_.CreateRootParameter("LightParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	spriteObjectRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

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

	// 色調補正のまとめ
	colorCorrectionRootParameter_.Initialize();
	colorCorrectionRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	colorCorrectionRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	colorCorrectionRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// グレースケールのまとめ
	grayScaleRootParameter_.Initialize();
	grayScaleRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	grayScaleRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	grayScaleRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ボックスフィルタのまとめ
	boxFilterRootParameter_.Initialize();
	boxFilterRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	boxFilterRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	boxFilterRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ビネットのまとめ
	vignetteRootParameter_.Initialize();
	vignetteRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	vignetteRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	vignetteRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ドット化のまとめ
	pixelRootParameter_.Initialize();
	pixelRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	pixelRootParameter_.CreateRootParameter("OffsetParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	pixelRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// 何もしないもの
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

	// スカイボックスのルートパラメータ
	skyBoxRootParameter_.Initialize();
	skyBoxRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	skyBoxRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	skyBoxRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	
	skyBoxRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// パラメータの整合性チェック
#ifdef QFE_OPTIMIZE_OFF
	normalGameObjectRootParameter_.CheckIntegrityData();
	spriteObjectRootParameter_.CheckIntegrityData();
	particleRootParameter_.CheckIntegrityData();
	primitiveRootParameter_.CheckIntegrityData();
	grayScaleRootParameter_.CheckIntegrityData();
	boxFilterRootParameter_.CheckIntegrityData();
	vignetteRootParameter_.CheckIntegrityData();
	normalRootParameter_.CheckIntegrityData();
	colorCorrectionRootParameter_.CheckIntegrityData();
	fontRootParameter_.CheckIntegrityData();
	skyBoxRootParameter_.CheckIntegrityData();
#endif // QFE_OPTIMIZE_OFF

	// インプットレイアウトの初期化
	normalInputLayout_.Initialize();
	normalInputLayout_.CreateNormalPresetInputLayout();
	primitiveInputLayout_.Initialize();
	primitiveInputLayout_.CreatePrimitivePresetInputLayout();

	// リフレクション情報からインプットレイアウトを生成
	// TODO: PSとVSの組み合わせによって変わる可能性があるため、もう一つ上のレイヤーが必要

	// PSOを作成
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	for (int i = 0; i < kCountOfBlendMode; i++) {
		trianglePso_[i].Initialize(&shaderCompiler_, device);
		trianglePso_[i].CreatePipelineStateObject(
			normalGameObjectRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", "Object3d.VS.hlsl", static_cast<BlendMode>(i), false);

		spritePso_[i].Initialize(&shaderCompiler_, device);
		spritePso_[i].CreatePipelineStateObject(
			spriteObjectRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Object2d.PS.hlsl", "Object2d.VS.hlsl", static_cast<BlendMode>(i), true);

		linePso_[i].Initialize(&shaderCompiler_, device);
		linePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i), true);

		pointPso_[i].Initialize(&shaderCompiler_, device);
		pointPso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i), true);

		primitivePso_[i].Initialize(&shaderCompiler_, device);
		primitivePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", "Primitive.VS.hlsl", static_cast<BlendMode>(i), true);

		particlePso_[i].Initialize(&shaderCompiler_, device);
		particlePso_[i].CreatePipelineStateObject(
			particleRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Particle.PS.hlsl", "Particle.VS.hlsl", static_cast<BlendMode>(i), false);
	}

	colorCorrectionPso_.Initialize(&shaderCompiler_, device);
	colorCorrectionPso_.CreatePipelineStateObject(
		colorCorrectionRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "ColorCorrectionShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	grayScaleTrianglePso_.Initialize(&shaderCompiler_, device);
	grayScaleTrianglePso_.CreatePipelineStateObject(
		grayScaleRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "GrayscaleShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);
	
	boxFilterPso_.Initialize(&shaderCompiler_, device);
	boxFilterPso_.CreatePipelineStateObject(
		boxFilterRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "BoxFilter.PS.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	vignettePso_.Initialize(&shaderCompiler_, device);
	vignettePso_.CreatePipelineStateObject(
		vignetteRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "VignetteShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	normalPso_.Initialize(&shaderCompiler_, device);
	normalPso_.CreatePipelineStateObject(
		grayScaleRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Simple.PS.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	pixelPso_.Initialize(&shaderCompiler_, device);
	pixelPso_.CreatePipelineStateObject(
		pixelRootParameter_, dxCommon->GetDepthStencilDesc(), normalInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "PixcelShader.hlsl", "Simple.VS.hlsl", kBlendModeNormal, false);

	fontPso_.Initialize(&shaderCompiler_, device);
	fontPso_.CreatePipelineStateObject(
		fontRootParameter_, dxCommon->GetDepthStencilDesc(), primitiveInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "MSDF.PS.hlsl", "Font.VS.hlsl", kBlendModeNormal, false);

	// Skybox用のDepthStencil設定(一番奥=1.0で描画されるようにLESS_EQUALにする)
	D3D12_DEPTH_STENCIL_DESC skyboxDepthStencilDesc = dxCommon->GetDepthStencilDesc();
	skyboxDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	skyboxDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	skyBoxPso_.Initialize(&shaderCompiler_, device);
	skyBoxPso_.CreatePipelineStateObject(
		skyBoxRootParameter_, skyboxDepthStencilDesc, normalInputLayout_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "SkyBox.PS.hlsl", "SkyBox.VS.hlsl", kBlendModeNormal, true);
}
