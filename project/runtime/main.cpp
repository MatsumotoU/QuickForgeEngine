#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "framework/D3D12GraphicFrameWork.h"
#include "framework/SceneFrameWork.h"
#include "gui/D3D12GuiManager.h"
#include "camera/CameraManager.h"
#include "scene/SceneManager.h"
#include "scene/component/AllComponent.h"
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
	std::string windowName = "Runtime Window";
	gameWindowManager->AddWindow(1280, 720, windowName);

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine =
		QFE::FRAMEWORK::CreateGraphicEngine(gameWindowManager->GetWindow(windowName));
	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager =
		QFE::FRAMEWORK::CreateGuiManager(graphicEngine.get(), gameWindowManager->GetWindow(windowName));

	// カメラマネージャの初期化とカメラの作成
	QFE::CAMERA::CameraManager cameraManager;
	cameraManager.Initialize();
	QFE::CAMERA::CameraHandle cameraHandle = cameraManager.CreateCamera(0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 100.0f, 0.45f);

	// シーンマネージャの初期化
	QFE::SCENE::SceneManager sceneManager;
	sceneManager.Initialize();

	// Entityの生成
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
	uint32_t entity = QFE::FRAMEWORK::CreateEntityWithMaterial(sceneManager, "RingObject", {1.0f, 0.0f, 0.0f, 1.0f});
	QFE::MATH::Transform& objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entity).transform;

	uint32_t floorEntity = QFE::FRAMEWORK::CreateEntityWithMaterial(sceneManager, "FloorObject", { 0.5f, 0.5f, 0.5f, 1.0f });
	QFE::MATH::Transform& floorTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(floorEntity).transform;
	floorTransform.translate = { 0.0f, -5.0f, 0.0f };
	floorTransform.scale = { 10.0f, 1.0f, 10.0f };

	std::string psDirName = "engine/resources/shaders/ps/";
	std::string vsDirName = "engine/resources/shaders/vs/";
	std::string rtDirName = "engine/resources/shaders/rt/";

	//====================
	// ここから描画の準備
	//====================

	// シェーダーペアを生成
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle =
		QFE::FRAMEWORK::CreateShaderPair(graphicEngine.get(), vsDirName, psDirName, "Object3d.GBuffer.VS.hlsl", "Object3d.GBuffer.PS.hlsl");
	// パイプラインステートオブジェクトを生成
	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->CreatePipelineStateObject(
		shaderPairHandle, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::Default);
	
	QFE::MATH::Transform cameraTransform;
	cameraTransform.translate = { 0.0f, 20.0f, -20.0f };
	cameraTransform.rotate = { 0.8f, 0.0f, 0.0f };

	// Vertexバッファの作成とモデルデータの読み込み
	QFE::ASSET::AssimpModelLoader modelLoader;
	modelLoader.Initialize();
	QFE::ASSET::ModelData& modelData = modelLoader.LoadModel("resources/box.obj");
	QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle = graphicEngine->CreateVertexBuffer(modelData.meshes[0].vertices.GetInternalVector(), "VertexBuffer");
	QFE::ASSET::ModelData& ringModelData = modelLoader.LoadModel("resources/ring.obj");
	QFE::GRAPHIC::DirectXResourceHandle ringVertexBufferHandle = graphicEngine->CreateVertexBuffer(ringModelData.meshes[0].vertices.GetInternalVector(), "VertexBuffer");

	// オフスクリーンレンダーターゲットの作成
	std::vector<QFE::GRAPHIC::RenderTargetHandle> renderTargets;
	for (int i = 0; i < 3; ++i) {
		QFE::GRAPHIC::RenderTargetHandle offScreenRenderTargetHandle =
			graphicEngine->CreateOffScreenRenderTarget(1280, 720, DXGI_FORMAT_R16G16B16A16_FLOAT);
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
	objectVertices = QFE::FRAMEWORK::GetModelVertexPositions(modelData.meshes[0].vertices.GetInternalVector());
	std::vector<QFE::MATH::Vector3> ringVertices;
	ringVertices = QFE::FRAMEWORK::GetModelVertexPositions(ringModelData.meshes[0].vertices.GetInternalVector());

	QFE::GRAPHIC::BLASHandle blasHandle = graphicEngine->CreateBLAS(
		objectVertices, "TriangleBLAS");
	QFE::GRAPHIC::BLASHandle ringBlasHandle = graphicEngine->CreateBLAS(
		ringVertices, "RingBLAS");

	QFE::GRAPHIC::BLASInstanceHandle blasInstanceHandle = graphicEngine->CreateBLASInstance(ringBlasHandle, QFE::MATH::Matrix4x4::MakeIndentity4x4());
	QFE::GRAPHIC::BLASInstanceHandle floorBlasInstanceHandle = graphicEngine->CreateBLASInstance(blasHandle, QFE::MATH::Matrix4x4::MakeIndentity4x4());

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

			// ルートリソースの設定
			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rootResources;
			QFE::FRAMEWORK::CreateObject3dGBufferRootResources(graphicEngine.get(), rootResources);
			std::vector<QFE::GRAPHIC::DirectXResourceHandle> floorRootResources;
			QFE::FRAMEWORK::CreateObject3dGBufferRootResources(graphicEngine.get(), floorRootResources);

			QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
			Material* ringMaterial = graphicEngine->GetConstantBufferData<Material>(rootResources[1]);
			ringMaterial->color = entityManager.GetComponent<QFE::SCENE::MaterialComponent>(entity).albedoColor;

			Material* floorMaterial = graphicEngine->GetConstantBufferData<Material>(floorRootResources[1]);
			floorMaterial->color = entityManager.GetComponent<QFE::SCENE::MaterialComponent>(floorEntity).albedoColor;

			graphicEngine->PreDraw();
			guiManager->PreDraw();

			
#ifdef USE_IMGUI
			ImGui::Begin("Test Window");
			ImGui::Text("Hello, world!");
			ImGui::DragFloat3("Object Scale", &objTransform.scale.x, 0.1f);
			ImGui::DragFloat3("Object Rotate", &objTransform.rotate.x, 0.1f);
			ImGui::DragFloat3("Object Translate", &objTransform.translate.x, 0.1f);
			ImGui::Separator();
			ImGui::DragFloat3("Floor Scale", &floorTransform.scale.x, 0.1f);
			ImGui::DragFloat3("Floor Rotate", &floorTransform.rotate.x, 0.1f);
			ImGui::DragFloat3("Floor Translate", &floorTransform.translate.x, 0.1f);
			ImGui::End();

			objTransform.rotate.x += 0.01f;
			objTransform.rotate.z += 0.01f;

			ImGui::Begin("Camera Window");
			ImGui::DragFloat3("Scale", &cameraTransform.scale.x, 0.1f);
			ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.1f);
			ImGui::DragFloat3("Translate", &cameraTransform.translate.x, 0.1f);
			ImGui::End();
#endif
			QFE::MATH::Matrix4x4 viewProj =
				cameraManager.GetViewProjectionMatrix(cameraHandle, cameraTransform, QFE::CAMERA::CameraType::Perspective);

			QFE::FRAMEWORK::UpdateObject3dWVPMatrix(graphicEngine.get(), rootResources[0], objTransform, viewProj);
			TransformationMatrix* transformMatrixData = graphicEngine->GetConstantBufferData<TransformationMatrix>(rootResources[0]);
			graphicEngine->UpdateBLASInstanceTransform(blasInstanceHandle, transformMatrixData->World);

			QFE::FRAMEWORK::UpdateObject3dWVPMatrix(graphicEngine.get(), floorRootResources[0], floorTransform, viewProj);
			TransformationMatrix* floorTransformMatrixData = graphicEngine->GetConstantBufferData<TransformationMatrix>(floorRootResources[0]);
			graphicEngine->UpdateBLASInstanceTransform(floorBlasInstanceHandle, floorTransformMatrixData->World);

			graphicEngine->TestOffScreenDraw(
				psoHandle, viewportHandle, scissorRectHandle, ringVertexBufferHandle, rootResources, renderTargets);
			graphicEngine->TestOffScreenDraw(
				psoHandle, viewportHandle, scissorRectHandle, vertexBufferHandle, floorRootResources, renderTargets);

			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources = {
				graphicEngine->GetRenderTargetTexture(renderTargets[0]),
				graphicEngine->GetRenderTargetTexture(renderTargets[1]),
				graphicEngine->GetRenderTargetTexture(renderTargets[2])
			};

			graphicEngine->TestRayTracing(rtpsoHandle, uavBufferHandle, rayTracingRootResources);

			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager->PostDraw();
			graphicEngine->PostDraw();
			sceneManager.EndFrame();
		}
	}

	sceneManager.SaveCurrentSceneToJson("resources/scene.json");

	sceneManager.Shutdown();

	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
	return 0;
}