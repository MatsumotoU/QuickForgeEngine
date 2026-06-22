#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "core/loger/MyDebugLog.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	std::unique_ptr<QFE::GameWindowManager> gameWindowManager = std::make_unique<QFE::GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(1280, 720, "Test Window");

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine = 
		std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(gameWindowManager->GetWindow("Test Window"));
	graphicEngine->Initialize();

	// ModelDataの作成
	QFE::ASSET::AssimpModelLoader modelLoader;
	QFE::ASSET::ModelData& modelData = modelLoader.LoadModel("resources/box.obj");


	QFE::GRAPHIC::ViewPortHandle viewPortHandle = graphicEngine->CreateViewPort(1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle = graphicEngine->CreateScissorRect(0, 0, 1280, 720);

	QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle = 
		graphicEngine->CreateVertexBuffer(modelData.meshes[0].vertices.GetInternalVector(), "model");

	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->GetBuiltInPipelineStateObject(
		QFE::GRAPHIC::BuiltInShaderPair::Object3D, QFE::GRAPHIC::BlendMode::kBlendModeNone,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::None);

	std::vector<QFE::GRAPHIC::DirectXResourceHandle> rootResources;
	// トランスフォーム行列の作成と定数バッファの作成
	QFE::MATH::Transform transform = {};
	transform.translate = { 0.0f, 0.0f, 0.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.scale = { 1.0f, 1.0f, 1.0f };
	TransformationMatrix transformMatrix = {};
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeAffineMatrix(transform);
	transformMatrix.WVP = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	QFE::GRAPHIC::DirectXResourceHandle constantBufferHandle = 
		graphicEngine->CreateConstantBuffer<TransformationMatrix>(transformMatrix, "TransformMatrix");
	rootResources.push_back(constantBufferHandle);

	Material material = {};
	material.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.enableLighting = 1;
	material.uvTransform = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	material.shininess = 32.0f;
	QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle = 
		graphicEngine->CreateConstantBuffer<Material>(material, "Material");
	rootResources.push_back(materialBufferHandle);
	
	DirectionalLight directionalLight = {};
	directionalLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLight.direction = { 0.0f, -1.0f, 0.0f };
	directionalLight.intensity = 1.0f;
	QFE::GRAPHIC::DirectXResourceHandle directionalLightBufferHandle = 
		graphicEngine->CreateConstantBuffer<DirectionalLight>(directionalLight, "DirectionalLight");
	rootResources.push_back(directionalLightBufferHandle);

	CameraForGPU camera = {};
	camera.cameraPosition = { 0.0f, 0.0f, -5.0f };
	QFE::GRAPHIC::DirectXResourceHandle cameraBufferHandle = 
		graphicEngine->CreateConstantBuffer<CameraForGPU>(camera, "Camera");
	rootResources.push_back(cameraBufferHandle);

	QFE::GRAPHIC::DirectXResourceHandle textureHandle = 
		graphicEngine->CreateTextureFromFile("resources/"+modelData.meshes[0].material.textureName);
	rootResources.push_back(textureHandle);

	QFE::GRAPHIC::DirectXResourceHandle cubeTextureHandle = 
		graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyBlackCubeMap);
	rootResources.push_back(cubeTextureHandle);

	// メインループ
	while (gameWindowManager->IsWindowActive()) {
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			// WM_QUITメッセージが来たらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			graphicEngine->PreDraw();

			// 描画処理
			//graphicEngine->GetConstantBufferData<TransformationMatrix>(constantBufferHandle)->World = QFE::MATH::Matrix4x4::MakeAffineMatrix(transform);
			graphicEngine->TestDraw(psoHandle,viewPortHandle, scissorRectHandle, vertexBufferHandle, rootResources);

			graphicEngine->PostDraw();
		}
	}

	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
