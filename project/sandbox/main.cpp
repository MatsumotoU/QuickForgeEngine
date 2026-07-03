#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "framework/D3D12GraphicFrameWork.h"
#include "gui/D3D12GuiManager.h"
#include "camera/CameraManager.h"
#include "core/loger/MyDebugLog.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

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
		QFE::FRAMEWORK::CreateGraphicEngine(gameWindowManager->GetWindow("Test Window"));
	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager = 
		QFE::FRAMEWORK::CreateGuiManager(graphicEngine.get(), gameWindowManager->GetWindow("Test Window"));

	// カメラマネージャの初期化とカメラの作成
	QFE::CAMERA::CameraManager cameraManager;
	cameraManager.Initialize();
	QFE::CAMERA::CameraHandle cameraHandle = cameraManager.CreateCamera(0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 100.0f, 0.45f);

	std::string psDirName = "engine/resources/shaders/ps/";
	std::string vsDirName = "engine/resources/shaders/vs/";
	std::string rtDirName = "engine/resources/shaders/rt/";

	//====================
	// ここから描画の準備
	//====================

	// シェーダーペアを生成
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle = 
		QFE::FRAMEWORK::CreateObject3dGBufferShaderPair(graphicEngine.get(), vsDirName, psDirName, "Object3d.VS.hlsl", "Object3d.GBuffer.PS.hlsl");
	// パイプラインステートオブジェクトを生成
	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->CreatePipelineStateObject(
		shaderPairHandle, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::Default);

	QFE::MATH::Transform objTransform;
	QFE::MATH::Transform cameraTransform;

	// Vertexバッファの作成とモデルデータの読み込み
	QFE::ASSET::AssimpModelLoader modelLoader;
	modelLoader.Initialize();
	QFE::ASSET::ModelData& modelData = modelLoader.LoadModel("resources/box.obj");
	QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle = graphicEngine->CreateVertexBuffer(modelData.meshes[0].vertices.GetInternalVector(), "VertexBuffer");
	// ルートリソースの設定
	std::vector<QFE::GRAPHIC::DirectXResourceHandle> rootResources;
	QFE::FRAMEWORK::CreateObject3dGBufferRootResources(graphicEngine.get(), rootResources);

	// オフスクリーンレンダーターゲットの作成
	std::vector<QFE::GRAPHIC::RenderTargetHandle> renderTargets;
	for (int i = 0; i < 3; ++i) {
		QFE::GRAPHIC::RenderTargetHandle offScreenRenderTargetHandle = graphicEngine->CreateOffScreenRenderTarget(1280, 720);
		renderTargets.push_back(offScreenRenderTargetHandle);
	}
	// ビューポートとシザー矩形の作成
	QFE::GRAPHIC::ViewPortHandle viewportHandle = graphicEngine->CreateViewPort(1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle = graphicEngine->CreateScissorRect(0, 0, 1280, 720);

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle = graphicEngine->CreateUAVBuffer(1280, 720, L"UAVBuffer");
	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::GRAPHIC::RTPSOHandle rtpsoHandle = graphicEngine->CreateRayTracingPipelineStateObject(
		rtDirName, "ShadowRaytracing.hlsl");

	// BLASの作成とBLASインスタンスの作成
	std::vector<QFE::MATH::Vector3> objectVertices;
	for (const auto& mesh : modelData.meshes) {
		objectVertices.clear();
		for (const auto& vertex : mesh.vertices) {
			objectVertices.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });
		}
	}

	QFE::GRAPHIC::BLASHandle blasHandle = graphicEngine->CreateBLAS(
		objectVertices,"TriangleBLAS");

	QFE::GRAPHIC::BLASInstanceHandle blasInstanceHandle = graphicEngine->CreateBLASInstance(blasHandle, QFE::MATH::Matrix4x4::MakeIndentity4x4());

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
			guiManager->PreDraw();


#ifdef USE_IMGUI
			ImGui::Begin("Test Window");
			ImGui::Text("Hello, world!");
			ImGui::DragFloat3("Object Scale", &objTransform.scale.x, 0.1f);
			ImGui::DragFloat3("Object Rotate", &objTransform.rotate.x, 0.1f);
			ImGui::DragFloat3("Object Translate", &objTransform.translate.x, 0.1f);
			ImGui::End();
#endif
			TransformationMatrix* transformMatrixData = graphicEngine->GetConstantBufferData<TransformationMatrix>(rootResources[0]);
			transformMatrixData->World = QFE::MATH::Matrix4x4::MakeAffineMatrix(objTransform);
			QFE::MATH::Matrix4x4 viewProj = 
				cameraManager.GetViewProjectionMatrix(cameraHandle, cameraTransform, QFE::CAMERA::CameraType::Perspective);
			transformMatrixData->WVP = QFE::MATH::Matrix4x4::Multiply(transformMatrixData->World, viewProj);

			graphicEngine->UpdateBLASInstanceTransform(blasInstanceHandle, transformMatrixData->World);

			graphicEngine->TestOffScreenDraw(
				psoHandle, viewportHandle, scissorRectHandle, vertexBufferHandle, rootResources, renderTargets);

			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources = {
				graphicEngine->GetRenderTargetTexture(renderTargets[0]),
				graphicEngine->GetRenderTargetTexture(renderTargets[1]),
				graphicEngine->GetRenderTargetTexture(renderTargets[2])
			};
			
			graphicEngine->TestRayTracing(rtpsoHandle, uavBufferHandle, rayTracingRootResources);
			
			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager->PostDraw();
			graphicEngine->PostDraw();
		}
	}

	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
