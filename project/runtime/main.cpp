#define NOMINMAX
#include <Windows.h>

#include "framework/QuickForgeEngineFrameWork.h"

#include "gui/D3D12GuiManager.h"
#include "camera/CameraManager.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "core/loger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	QFE::FRAMEWORK::WindowsEngineSystems engineSystems;
	QFE::FRAMEWORK::EngineConfigDesc engineConfigDesc;
	engineConfigDesc.mainWindowName = "Runtime";
	engineConfigDesc.mainWindowHeight = 720;
	engineConfigDesc.mainWindowWidth = 1280;
	// エンジンのシステムの初期化
	QFE::FRAMEWORK::InitializeEngine(engineSystems, engineConfigDesc);
	HWND mainWindow = engineSystems.windowManager->GetWindow(engineConfigDesc.mainWindowName);
	
	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager =
		QFE::FRAMEWORK::CreateGuiManager(engineSystems.graphicEngine.get(), mainWindow);

	// FPSカウンターの初期化
	std::unique_ptr<QFE::FPSCounter> fpsCounter = std::make_unique<QFE::FPSCounter>();
	fpsCounter->Reset();

	// カメラマネージャの初期化とカメラの作成
	QFE::CAMERA::CameraManager cameraManager;
	cameraManager.Initialize();
	QFE::CAMERA::CameraHandle cameraHandle = cameraManager.CreateCamera(0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 100.0f, 0.45f);

	// シーンマネージャの初期化
	QFE::SCENE::SceneManager sceneManager;
	sceneManager.Initialize();
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	// JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
	std::wstring selectedFilePath;
	if (QFE::WINDOW::RequestGetFilePathFromUser(
		mainWindow,
		L"JSON Files", L"*.json",
		selectedFilePath)){
		// Entityの生成
		sceneManager.LoadCurrentSceneFromJson(QFE::ConvertString(selectedFilePath));
	}
	

	std::string psDirName = "engine/resources/shaders/ps/";
	std::string vsDirName = "engine/resources/shaders/vs/";
	std::string rtDirName = "engine/resources/shaders/rt/";

	//====================
	// ここから描画の準備
	//====================

	// シェーダーペアを生成
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle =
		QFE::FRAMEWORK::CreateShaderPair(engineSystems.graphicEngine.get(), vsDirName, psDirName, "Object3d.GBuffer.VS.hlsl", "Object3d.GBuffer.PS.hlsl");
	// パイプラインステートオブジェクトを生成
	QFE::GRAPHIC::PSOHandle psoHandle = engineSystems.graphicEngine->CreatePipelineStateObject(
		shaderPairHandle, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::Default);

	QFE::MATH::Transform cameraTransform;
	cameraTransform.translate = { 0.0f, 20.0f, -20.0f };
	cameraTransform.rotate = { 0.8f, 0.0f, 0.0f };

	// Vertexバッファの作成とモデルデータの読み込み
	QFE::ASSET::AssimpModelLoader modelLoader;
	modelLoader.Initialize();
	std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle> vertexBufferMap;
	std::unordered_map<std::string, QFE::ASSET::ModelData> modelDataMap;
	std::vector<std::string> modelNames;
	entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
		modelNames.push_back(modelRenderComp.modelName);
		});
	std::string modelDir = "resources/";
	for (std::string modelName : modelNames) {
		modelDataMap[modelName] = modelLoader.LoadModel(modelDir + modelName + ".obj");
		vertexBufferMap[modelName] = engineSystems.graphicEngine->CreateVertexBuffer(modelDataMap[modelName].meshes[0].vertices.GetInternalVector(), "VertexBuffer");
	}

	// オフスクリーンレンダーターゲットの作成
	std::vector<QFE::GRAPHIC::RenderTargetHandle> renderTargets;
	for (int i = 0; i < 3; ++i) {
		QFE::GRAPHIC::RenderTargetHandle offScreenRenderTargetHandle =
			engineSystems.graphicEngine->CreateOffScreenRenderTarget(1280, 720, DXGI_FORMAT_R16G16B16A16_FLOAT);
		renderTargets.push_back(offScreenRenderTargetHandle);
	}
	// ビューポートとシザー矩形の作成
	QFE::GRAPHIC::ViewPortHandle viewportHandle = engineSystems.graphicEngine->CreateViewPort(1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle = engineSystems.graphicEngine->CreateScissorRect(0, 0, 1280, 720);

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle = engineSystems.graphicEngine->CreateUAVBuffer(1280, 720, L"UAVBuffer");
	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::GRAPHIC::RTPSOHandle rtpsoHandle = engineSystems.graphicEngine->CreateRayTracingPipelineStateObject(
		rtDirName, "ShadowRaytracing.hlsl");

	// BLASの作成とBLASインスタンスの作成
	std::unordered_map<std::string, QFE::GRAPHIC::BLASHandle> blasHandleMap;
	for (const auto& [modelName, modelData] : modelDataMap) {
		std::vector<QFE::MATH::Vector3> objectVertices;
		objectVertices = QFE::FRAMEWORK::GetModelVertexPositions(modelData.meshes[0].vertices.GetInternalVector());
		QFE::GRAPHIC::BLASHandle blasHandle = engineSystems.graphicEngine->CreateBLAS(
			objectVertices, modelName);
		blasHandleMap[modelName] = blasHandle;
	}

	// メインループ
	while (engineSystems.windowManager->IsWindowActive()) {
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			// WM_QUITメッセージが来たらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			fpsCounter->FrameStart();

			QFE::MATH::Matrix4x4 viewProj =
				cameraManager.GetViewProjectionMatrix(cameraHandle, cameraTransform, QFE::CAMERA::CameraType::Perspective);

			// 各エンティティのModelRenderComponentを更新
			std::vector<QFE::GRAPHIC::RaytracingInstance> raytracingInstances;
			entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
				modelRenderComp.canRender = false;
				// TransformComponentを取得して、Transformを更新する
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId) == false) {
					return;
				}
				QFE::MATH::Transform& objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
				QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = engineSystems.graphicEngine->GetResourceAllocator();
				QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
					resourceAllocator->AllocateConstantBuffer<TransformationMatrix>();
				QFE::FRAMEWORK::UpdateObject3dWVPMatrix(engineSystems.graphicEngine.get(), transformMatrixBufferHandle, objTransform, viewProj);
				modelRenderComp.transformMatrixBufferHandle =
					static_cast<uint32_t>(transformMatrixBufferHandle);

				// マテリアルの更新
				if (entityManager.HasComponent<QFE::SCENE::MaterialComponent>(entityId) == false) {
					return;
				}
				QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle =
					resourceAllocator->AllocateConstantBuffer<Material>();
				Material* materialData = engineSystems.graphicEngine->GetConstantBufferData<Material>(materialBufferHandle);
				QFE::SCENE::MaterialComponent& materialComp = entityManager.GetComponent<QFE::SCENE::MaterialComponent>(entityId);
				materialData->color = materialComp.albedoColor;
				modelRenderComp.materialResourceHandle = static_cast<uint32_t>(materialBufferHandle);

				// 頂点バッファの更新
				if (vertexBufferMap.find(modelRenderComp.modelName) != vertexBufferMap.end()) {
					modelRenderComp.vertexResourceHandle = static_cast<uint32_t>(vertexBufferMap[modelRenderComp.modelName]);
				} else {
					return;
				}

				// テクスチャの更新
				QFE::GRAPHIC::DirectXResourceHandle textureHandle =
					engineSystems.graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyWhite1x1Texture);
				modelRenderComp.textureResourceHandle = static_cast<uint32_t>(textureHandle);

				// レイトレーシングインスタンスの作成
				raytracingInstances.push_back({
					blasHandleMap[modelRenderComp.modelName],
					QFE::MATH::Matrix4x4::MakeAffineMatrix(entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform)
					});

				// レンダリング可能
				modelRenderComp.canRender = true;
				});

			engineSystems.graphicEngine->UpdateBLASInstanceTransform(raytracingInstances);

			engineSystems.graphicEngine->PreDraw();
			guiManager->PreDraw();


#ifdef USE_IMGUI
			ImGui::Begin("Camera Window");
			ImGui::DragFloat3("Scale", &cameraTransform.scale.x, 0.1f);
			ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.1f);
			ImGui::DragFloat3("Translate", &cameraTransform.translate.x, 0.1f);
			ImGui::End();

			ImGui::Begin("Scene Window");
			ImGui::Text("SceneFPS: %.1f", fpsCounter->GetAverageFPS());
			if (ImGui::Button("Save Scene")) {
				// JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
				std::wstring selectedFilePath;
				if (QFE::WINDOW::RequestGetFilePathFromUser(
					mainWindow,
					L"JSON Files", L"*.json",
					selectedFilePath)) {
					// Entityの生成
					sceneManager.SaveCurrentSceneToJson(QFE::ConvertString(selectedFilePath));
				}
			}
			if (ImGui::Button("Load Scene")) {
				// JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
				std::wstring selectedFilePath;
				if (QFE::WINDOW::RequestGetFilePathFromUser(
					mainWindow,
					L"JSON Files", L"*.json",
					selectedFilePath)) {
					// Entityの生成
					sceneManager.LoadCurrentSceneFromJson(QFE::ConvertString(selectedFilePath));
				}
			}
			ImGui::End();

			ImGui::Begin("Entity Window");

			// スタティック変数で「現在どのエンティティを選択しているか」を保持（未選択は-1）
			static int selectedEntityId = -1;

			// ==========================================
			// 左側：エンティティ一覧（リスト）ペイン
			// ==========================================
			// 幅 180 ピクセル、高さは自動（0）の独立したスクロール領域を作る
			ImGui::BeginChild("EntityList", ImVec2(180, 0), true);
			ImGui::Text("Hierarchy");
			ImGui::Separator();

			std::vector<uint32_t> entityIds = entityManager.GetActiveEntityIds();
			for (uint32_t entityId : entityIds) {
				// 表示用ラベルの決定（ObjectInfoComponentがあればその名前、なければID）
				std::string label = "Entity " + std::to_string(entityId);
				if (entityManager.HasComponent<QFE::SCENE::ObjectInfoComponent>(entityId)) {
					label = entityManager.GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId).name + "##" + std::to_string(entityId);
				}


				bool isSelected = (selectedEntityId == static_cast<int>(entityId));

				// 1. 通常のクリック選択処理
				if (ImGui::Selectable(label.c_str(), isSelected)) {
					selectedEntityId = static_cast<int>(entityId);
				}

				// 2. ダブルクリックの判定（Selectable の直後に書くことで、そのアイテムに対して判定する）
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
						selectedEntityId = static_cast<int>(entityId); // 念のため選択状態も更新

						QFE::MATH::Transform& objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
						cameraTransform.translate = objTransform.translate + QFE::MATH::Vector3(0.0f, 5.0f, -10.0f);
						cameraTransform.rotate = QFE::MATH::Vector3(0.5f, 0.0f, 0.0f);
					}
				}
			}
			ImGui::EndChild();

			// 横並びにするために SameLine を呼ぶ
			ImGui::SameLine();

			// ==========================================
			// 右側：コンポーネント詳細（インスペクター）ペイン
			// ==========================================
			// 残りの領域すべてを使う（幅0, 高さ0）
			ImGui::BeginChild("ComponentInspector", ImVec2(0, 0), true);
			if (selectedEntityId != -1 && entityManager.IsActiveEntity(selectedEntityId)) {
				uint32_t currentId = static_cast<uint32_t>(selectedEntityId);

				// --- Object Info ---
				if (entityManager.HasComponent<QFE::SCENE::ObjectInfoComponent>(currentId)) {
					if (ImGui::CollapsingHeader("Object Info", ImGuiTreeNodeFlags_DefaultOpen)) {
						QFE::SCENE::ObjectInfoComponent& objInfoComp = entityManager.GetComponent<QFE::SCENE::ObjectInfoComponent>(currentId);
						ImGui::Text("Name: %s", objInfoComp.name.c_str());
						ImGui::Text("Entity ID: %d", currentId);
					}
				}

				// --- Transform ---
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(currentId)) {
					if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
						QFE::MATH::Transform& objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(currentId).transform;
						// 選択が固定されているので、ラベルに毎回IDを埋め込まなくても被り（競合）が起きません
						ImGui::DragFloat3("Scale", &objTransform.scale.x, 0.1f);
						ImGui::DragFloat3("Rotate", &objTransform.rotate.x, 0.1f);
						ImGui::DragFloat3("Translate", &objTransform.translate.x, 0.1f);
					}
				}

				// --- Material ---
				if (entityManager.HasComponent<QFE::SCENE::MaterialComponent>(currentId)) {
					if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
						QFE::SCENE::MaterialComponent& materialComp = entityManager.GetComponent<QFE::SCENE::MaterialComponent>(currentId);
						ImGui::ColorEdit3("Albedo Color", &materialComp.albedoColor.x);
					}
				}

				// --- Model Render ---
				if (entityManager.HasComponent<QFE::SCENE::ModelRenderComponent>(currentId)) {
					if (ImGui::CollapsingHeader("Model Render", ImGuiTreeNodeFlags_DefaultOpen)) {
						QFE::SCENE::ModelRenderComponent& modelRenderComp = entityManager.GetComponent<QFE::SCENE::ModelRenderComponent>(currentId);
						ImGui::Text("Model Name: %s", modelRenderComp.modelName.c_str());
						ImGui::Text("Can Render: %s", modelRenderComp.canRender ? "True" : "False");
					}
				}

			} else {
				// 何も選択されていないときのプレースホルダー
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select an entity from the hierarchy.");
			}
			ImGui::EndChild();

			ImGui::End();
#endif

			// モデルのレンダリング
			entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
				if (modelRenderComp.canRender == false) {
					return;
				}


				// ルートリソースの設定
				std::vector<QFE::GRAPHIC::DirectXResourceHandle> modelRootResources = {
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.transformMatrixBufferHandle),
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.materialResourceHandle),
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.textureResourceHandle)
				};
				engineSystems.graphicEngine->TestOffScreenDraw(
					psoHandle, viewportHandle, scissorRectHandle,
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.vertexResourceHandle),
					modelRootResources, renderTargets);
				});

			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources = {
				engineSystems.graphicEngine->GetRenderTargetTexture(renderTargets[0]),
				engineSystems.graphicEngine->GetRenderTargetTexture(renderTargets[1]),
				engineSystems.graphicEngine->GetRenderTargetTexture(renderTargets[2])
			};

			engineSystems.graphicEngine->TestRayTracing(rtpsoHandle, uavBufferHandle, rayTracingRootResources);

			engineSystems.graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager->PostDraw();
			engineSystems.graphicEngine->PostDraw();
			sceneManager.EndFrame();

			fpsCounter->FrameEnd();
		}
	}



	sceneManager.Shutdown();
	guiManager->Shutdown();
	QFE::FRAMEWORK::FinalizeEngine(engineSystems);

	return 0;
}