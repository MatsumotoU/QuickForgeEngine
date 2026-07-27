#include "WindowsEngineFramework.h"
#include "EngineDefines.h"

// 機能をunique_ptrをつかって管理するために必要なヘッダーファイル
#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "gui/D3D12GuiManager.h"
#include "input/InputInterface.h"
#include "scene/SceneManager.h"
#include "script/ScriptInstance.h"
#include "components/AllComponent.h"

bool QFE::FRAMEWORK::CreateWindowsQuickForgeEngineSystems(
	HINSTANCE hInstance, 
	const std::string& mainWindowName, uint32_t mainWindowWidth, uint32_t mainWindowHeight,
	WindowsQuickForgeEngineSystems& outSystems) {

	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	outSystems.windowManager = 
		QFE::FRAMEWORK::CreateWindowManager(mainWindowName, mainWindowWidth, mainWindowHeight);
	// ウィンドウマネージャの初期化に失敗した場合はエラーを報告して終了
	if(outSystems.windowManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create GameWindowManager.",QFE::SystemError::Abort);
		return false;
	}
	// メインウィンドウのハンドルを取得
	HWND mainWindow = QFE::FRAMEWORK::GetWindowHandle(outSystems.windowManager.get(), mainWindowName);

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	outSystems.graphicEngine =
		QFE::FRAMEWORK::CreateGraphicEngine(mainWindow);
	// グラフィックエンジンの初期化に失敗した場合はエラーを報告して終了
	if(outSystems.graphicEngine == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create D3D12GraphicEngine.",QFE::SystemError::Abort);
		return false;
	}

	outSystems.guiManager = QFE::FRAMEWORK::CreateGuiManager(outSystems.graphicEngine.get(), mainWindow);
	if (outSystems.guiManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create D3D12GuiManager.", QFE::SystemError::Abort);
		return false;
	}

	// InputInterfaceの初期化
	outSystems.inputInterface = QFE::FRAMEWORK::CreateInputInterface(mainWindow, hInstance);
	// InputInterfaceの初期化に失敗した場合はエラーを報告して終了
	if (outSystems.inputInterface == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create InputInterface.", QFE::SystemError::Abort);
		return false;
	}

	// FPSカウンターの初期化
	outSystems.fpsCounter = std::make_unique<QFE::FPSCounter>();
	outSystems.fpsCounter->Reset();

	// シーンマネージャの初期化
	outSystems.sceneManager = std::make_unique<QFE::SCENE::SceneManager>();
	outSystems.sceneManager->Initialize();

	// モデルローダーの初期化
	outSystems.modelLoader = std::make_unique<QFE::ASSET::AssimpModelLoader>();
	outSystems.modelLoader->Initialize();

	return true;
}

bool QFE::FRAMEWORK::ProcessWindowsApplicationMessage() {
	MSG msg{};
	if (!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		return true;
	}
	if (msg.message == WM_QUIT) {
		return false;
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	return true;
}

bool QFE::FRAMEWORK::BeginWindowsEngineFrame(WindowsQuickForgeEngineSystems& systems) {
	if (!systems.fpsCounter || !systems.inputInterface) {
		return false;
	}
	systems.fpsCounter->FrameStart();
	systems.inputInterface->Update();
	return true;
}

void QFE::FRAMEWORK::EndWindowsEngineFrame(WindowsQuickForgeEngineSystems& systems) {
	if (systems.guiManager) systems.guiManager->PostDraw();
	if (systems.graphicEngine) systems.graphicEngine->PostDraw();
	if (systems.sceneManager) systems.sceneManager->EndFrame();
	if (systems.fpsCounter) systems.fpsCounter->FrameEnd();
	if (systems.inputInterface) systems.inputInterface->EndFrame();
}

void QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(WindowsQuickForgeEngineSystems& systems) {
	if (systems.sceneManager) systems.sceneManager->Shutdown();
	if (systems.guiManager) systems.guiManager->Shutdown();
	if (systems.graphicEngine) systems.graphicEngine->Shutdown();
	if (systems.windowManager) systems.windowManager->Shutdown();
}

void QFE::FRAMEWORK::EngineInitialize(WindowsQuickForgeEngineSystems& systems, WindowsEngineResources& resources) {
	auto& gameWindowManager = systems.windowManager;
	auto& graphicEngine = systems.graphicEngine;
	auto& guiManager = systems.guiManager;
	auto& inputInterface = systems.inputInterface;
	auto& fpsCounter = systems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *systems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	HWND mainWindow = QFE::FRAMEWORK::GetWindowHandle(systems.windowManager.get(), resources.windowName);
	std::string psDirName = resources.psDirName;
	std::string vsDirName = resources.vsDirName;
	std::string rtDirName = resources.rtDirName;
	std::string csDirName = resources.csDirName;

	// JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
	std::wstring selectedFilePath;
	if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
		mainWindow,
		L"JSON Files", L"*.json",
		selectedFilePath)) {
		// Entityの生成
		systems.sceneManager->LoadCurrentSceneFromJson(QFE::ConvertString(selectedFilePath));
	}

	//====================
	// ここから描画の準備
	//====================

	float allObjectMetallic = 0.0f;
	float allObjectSmoothness = 0.5f;

	// シェーダーペアを生成
	QFE::FRAMEWORK::CreateShaderPair(graphicEngine.get(), vsDirName, psDirName, "Object3d.GBuffer.VS.hlsl", "Object3d.GBuffer.PS.hlsl", resources.shaderPairHandle);
	// パイプラインステートオブジェクトを生成
	
	QFE::FRAMEWORK::CreateGraphicPSO(graphicEngine.get(), resources.shaderPairHandle,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::DepthStencilDescType::Default, resources.psoHandle);
	QFE::FRAMEWORK::GetGraphicPSORootParameterTypeList(graphicEngine.get(), resources.psoHandle, resources.rootParameterTypes);

	QFE::FRAMEWORK::GetBlackCubeMapTextureHandle(graphicEngine.get(), resources.textureHandleMap["BlackCubeMap"]);
	resources.textureGpuIndexMap["BlackCubeMap"] = 0; // GPU側のインデックスを設定
	QFE::FRAMEWORK::GetWhite1x1TextureHandle(graphicEngine.get(), resources.textureHandleMap["White1x1"]);
	resources.textureGpuIndexMap["White1x1"] = 1; // GPU側のインデックスを設定

	// オフスクリーンレンダーターゲットの作成
	for (int i = 0; i < 4; ++i) {
		QFE::GRAPHIC::RenderTargetHandle offScreenRenderTargetHandle;
		QFE::FRAMEWORK::CreateOffScreenRenderTarget(
			graphicEngine.get(), offScreenRenderTargetHandle, 1280, 720, DXGI_FORMAT_R16G16B16A16_FLOAT);
		resources.renderTargets.push_back(offScreenRenderTargetHandle);
	}

	// ビューポートとシザー矩形の作成
	QFE::FRAMEWORK::CreateViewport(graphicEngine.get(), resources.viewportHandle, 1280, 720);
	QFE::FRAMEWORK::CreateScissorRect(graphicEngine.get(), resources.scissorRectHandle, 0, 0, 1280, 720);

	// UAVバッファの作成とルートリソースの設定
	QFE::FRAMEWORK::CreateUAVBuffer(graphicEngine.get(), resources.uavBufferHandle, 1280, 720, L"UAVBuffer");

	// GPUパーティクル用Compute PSOと、両シェーダーで共有するUAVを作成
	QFE::FRAMEWORK::CreateComputePSO(
		graphicEngine.get(), csDirName, "EmitParticle.CS.hlsl", resources.emitParticlePSOHandle);
	QFE::FRAMEWORK::CreateComputePSO(
		graphicEngine.get(), csDirName, "ParticleUpdate.CS.hlsl", resources.updateParticlePSOHandle);
	QFE::FRAMEWORK::CreateStructuredUAVBuffer(
		graphicEngine.get(), resources.particleBufferHandle,
		WindowsEngineResources::kParticleCount, sizeof(Particle), L"ParticleBuffer");
	QFE::FRAMEWORK::CreateStructuredUAVBuffer(
		graphicEngine.get(), resources.freeCounterBufferHandle, 1, sizeof(int32_t), L"ParticleFreeCounter");

	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::FRAMEWORK::CreateRayTracingPSO(graphicEngine.get(), resources.rtpsoHandle, rtDirName, "ShadowRaytracing.hlsl");
}

void QFE::FRAMEWORK::EnginePreDraw(WindowsQuickForgeEngineSystems& systems, WindowsEngineResources& resources) {
	auto& gameWindowManager = systems.windowManager;
	auto& graphicEngine = systems.graphicEngine;
	auto& guiManager = systems.guiManager;
	auto& inputInterface = systems.inputInterface;
	auto& fpsCounter = systems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *systems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	// 各エンティティのModelRenderComponentを更新
	std::vector<std::pair<QFE::GRAPHIC::BLASHandle, QFE::MATH::Matrix4x4>> raytracingInstances;
	entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
		modelRenderComp.canRender = false;
		// TransformComponentを取得して、EulerTransformを更新する
		if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId) == false) {
			modelRenderComp.renderErrorMessage = "Missing TransformComponent for entity: " + std::to_string(entityId);
			return;
		}
		QFE::MATH::EulerTransform objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;

		if (entityManager.HasComponent < QFE::SCENE::AnimationComponent>(entityId)) {
			QFE::SCENE::AnimationComponent& animationComp = entityManager.GetComponent<QFE::SCENE::AnimationComponent>(entityId);
			objTransform.translate += animationComp.transform.translate;
			objTransform.rotate += animationComp.transform.rotate;
			objTransform.scale += animationComp.transform.scale;
		}

		QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetDirectXResourceAllocator();
		QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
			resourceAllocator->AllocateConstantBuffer<TransformationMatrix>();
		QFE::FRAMEWORK::UpdateObject3dWVPMatrix(graphicEngine.get(), transformMatrixBufferHandle, objTransform, resources.viewProj);
		modelRenderComp.transformMatrixBufferHandle =
			static_cast<uint32_t>(transformMatrixBufferHandle);

		// マテリアルの更新
		if (entityManager.HasComponent<QFE::SCENE::MaterialComponent>(entityId) == false) {
			modelRenderComp.renderErrorMessage = "Missing MaterialComponent for entity: " + std::to_string(entityId);
			return;
		}
		QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle =
			resourceAllocator->AllocateConstantBuffer<Material>();
		Material* materialData = graphicEngine->GetConstantBufferData<Material>(materialBufferHandle);
		QFE::SCENE::MaterialComponent& materialComp = entityManager.GetComponent<QFE::SCENE::MaterialComponent>(entityId);
		materialData->color = materialComp.albedoColor;
		materialData->metallic = materialComp.metallic;
		materialData->smoothness = materialComp.smoothness;
		materialData->uvTransform = QFE::MATH::Matrix4x4::MakeAffineMatrix(materialComp.uvTransform);
		modelRenderComp.materialResourceHandle = static_cast<uint32_t>(materialBufferHandle);

		// 頂点バッファの更新
		if (resources.vertexBufferMap.find(modelRenderComp.modelName) != resources.vertexBufferMap.end()) {
			modelRenderComp.vertexResourceHandle = static_cast<uint32_t>(resources.vertexBufferMap[modelRenderComp.modelName]);
		} else {
			if (QFE::FRAMEWORK::LoadModelVertexData(systems, resources.modelDir, modelRenderComp.modelName, resources.modelDataMap, resources.vertexBufferMap)) {
				modelRenderComp.vertexResourceHandle = static_cast<uint32_t>(resources.vertexBufferMap[modelRenderComp.modelName]);
			} else {
				modelRenderComp.renderErrorMessage = "Failed to load vertex buffer for model: " + modelRenderComp.modelName;
				return;
			}
		}

		// インデックスバッファの更新
		if (resources.indexBufferMap.find(modelRenderComp.modelName) != resources.indexBufferMap.end()) {
			modelRenderComp.indexResourceHandle = static_cast<uint32_t>(resources.indexBufferMap[modelRenderComp.modelName]);
		} else {
			if (QFE::FRAMEWORK::LoadModelIndexBuffer(systems, resources.modelDir, modelRenderComp.modelName, resources.modelDataMap, resources.indexBufferMap)) {
				modelRenderComp.indexResourceHandle = static_cast<uint32_t>(resources.indexBufferMap[modelRenderComp.modelName]);
			} else {
				modelRenderComp.renderErrorMessage = "Failed to load index buffer for model: " + modelRenderComp.modelName;
				return;
			}
		}

		// テクスチャの更新
		QFE::GRAPHIC::DirectXResourceHandle textureHandle;
		QFE::FRAMEWORK::GetWhite1x1TextureHandle(graphicEngine.get(), textureHandle);
		if (resources.modelDataMap.find(modelRenderComp.modelName) != resources.modelDataMap.end()) {
			const QFE::ASSET::ModelData& modelData = resources.modelDataMap[modelRenderComp.modelName];
			if (!modelData.meshes.empty() && !modelData.meshes[0].material.textureName.empty()) {
				const std::string& textureFileName = modelData.meshes[0].material.textureName;
				if (QFE::FRAMEWORK::LoadTexture(systems, resources.assetDir, textureFileName, resources.textureHandleMap, resources.textureGpuIndexMap, resources.nextTextureGpuIndex)) {
					textureHandle = resources.textureHandleMap[textureFileName];
				} else {
					modelRenderComp.renderErrorMessage = "Failed to load texture: " + textureFileName;
					return;
				}
			}
		}
		modelRenderComp.textureResourceHandle = static_cast<uint32_t>(textureHandle);

		// レイトレーシングインスタンスの作成
		if (resources.blasHandleMap.find(modelRenderComp.modelName) == resources.blasHandleMap.end()) {
			if (!QFE::FRAMEWORK::LoadModelAndCreateBLAS(systems, resources.modelDir, modelRenderComp.modelName, resources.modelDataMap, resources.blasHandleMap)) {
				modelRenderComp.renderErrorMessage = "Failed to load BLAS for model: " + modelRenderComp.modelName;
				return;
			}
		}
		raytracingInstances.push_back({
			resources.blasHandleMap[modelRenderComp.modelName],
			QFE::MATH::Matrix4x4::MakeAffineMatrix(entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform)
			});

		// レンダリング可能
		modelRenderComp.canRender = true;
		modelRenderComp.renderErrorMessage = "";
		});

	// 1) global arrays とモデル→meta マップを作る
	std::vector<float> globalUVs;
	std::vector<uint32_t> globalTriIndices;
	std::unordered_map<std::string, InstanceMetaCPU> modelMetaMap;

	// modelDataMap に基づいて平坦化（models -> global arrays）
	// BuildGlobalMeshBuffers は modelName -> InstanceMeta を返す
	QFE::FRAMEWORK::BuildGlobalMeshBuffers(
		resources.modelDataMap, resources.textureGpuIndexMap, globalUVs, globalTriIndices, modelMetaMap);

	// 2) raytracingInstances の順に合わせて instanceMeta を並べる
	std::vector<InstanceMetaCPU> instanceMetaAligned;
	instanceMetaAligned.reserve(raytracingInstances.size());

	// 逆引きテーブル：BLASHandle -> modelName
	std::unordered_map<QFE::GRAPHIC::BLASHandle, std::string> blasToModel;
	for (const auto& kv : resources.blasHandleMap) {
		blasToModel[kv.second] = kv.first;
	}

	for (const auto& inst : raytracingInstances) {
		QFE::GRAPHIC::BLASHandle blas = inst.first;
		auto it = blasToModel.find(blas);
		if (it == blasToModel.end()) {
			// 見つからないなら安全なデフォルトを push（デバッグ用ログ推奨）
			InstanceMetaCPU dummy{};
			instanceMetaAligned.push_back(dummy);
			continue;
		}
		const std::string& modelName = it->second;
		auto mit = modelMetaMap.find(modelName);
		if (mit == modelMetaMap.end()) {
			InstanceMetaCPU dummy{};
			instanceMetaAligned.push_back(dummy);
			continue;
		}
		instanceMetaAligned.push_back(mit->second);
	}

	// 3) バッファのサイズ計算と EnsureBufferCapacityAndUpload による使い回しアップロード
	size_t uvBytes = globalUVs.size() * sizeof(float);
	size_t triBytes = globalTriIndices.size() * sizeof(uint32_t);
	size_t metaBytes = instanceMetaAligned.size() * sizeof(InstanceMetaCPU);

	if (!QFE::FRAMEWORK::EnsureBufferCapacityAndUpload(graphicEngine.get(), resources.globalUVHandle, globalUVs.data(), uvBytes, sizeof(float) * 2, "GlobalUVs")) {
		assert(false && "Failed to ensure/upload GlobalUVs");
	}
	if (!QFE::FRAMEWORK::EnsureBufferCapacityAndUpload(graphicEngine.get(), resources.globalTriHandle, globalTriIndices.data(), triBytes, sizeof(uint32_t) * 3, "GlobalTriIndices")) {
		assert(false && "Failed to ensure/upload GlobalTriIndices");
	}
	if (!QFE::FRAMEWORK::EnsureBufferCapacityAndUpload(graphicEngine.get(), resources.instanceMetaHandle, instanceMetaAligned.data(), metaBytes, sizeof(InstanceMetaCPU), "InstanceMeta")) {
		assert(false && "Failed to ensure/upload InstanceMeta");
	}

	// TLAS 更新、描画へ進む
	QFE::FRAMEWORK::UpdateBLASInstanceBuffer(graphicEngine.get(), raytracingInstances);

	graphicEngine->PreDraw();
	guiManager->PreDraw();

	// Compute用CBVはフレームアロケータから毎フレーム確保する。
	auto* allocator = graphicEngine->GetDirectXResourceAllocator();
	resources.particleCountBufferHandle = allocator->AllocateConstantBuffer<uint32_t>("ParticleCount");
	resources.emitterBufferHandle = allocator->AllocateConstantBuffer<EmitterSphere>("ParticleEmitter");
	resources.particlePerFrameBufferHandle = allocator->AllocateConstantBuffer<PerFrame>("ParticlePerFrame");

	*graphicEngine->GetConstantBufferData<uint32_t>(resources.particleCountBufferHandle) =
		WindowsEngineResources::kParticleCount;
	auto* emitter = graphicEngine->GetConstantBufferData<EmitterSphere>(resources.emitterBufferHandle);
	*emitter = {};
	emitter->translate = { 0.0f, 0.0f, 0.0f };
	emitter->radius = 1.0f;
	emitter->count = 8;
	emitter->emit = 1;
	auto* perFrame = graphicEngine->GetConstantBufferData<PerFrame>(resources.particlePerFrameBufferHandle);
	*perFrame = {};
	perFrame->deltaTime = fpsCounter->GetDeltaTime();
	resources.particleElapsedTime += perFrame->deltaTime;
	perFrame->time = resources.particleElapsedTime;

	using QFE::GRAPHIC::ViewTypeFlags;
	const ViewTypeFlags cbv = ViewTypeFlags::ConstantBufferView;
	const ViewTypeFlags uav = ViewTypeFlags::UnorderedAccessView;
	QFE::FRAMEWORK::DispatchCompute(
		graphicEngine.get(), resources.emitParticlePSOHandle,
		{ resources.emitterBufferHandle, resources.particlePerFrameBufferHandle,
		  resources.particleBufferHandle, resources.freeCounterBufferHandle },
		{ cbv, cbv, uav, uav }, 1);

	// Emitが書き込んだParticleをUpdateが同じフレームで読むため、UAVアクセスを順序付ける。
	QFE::FRAMEWORK::UAVBarrierTransition(graphicEngine.get(), resources.particleBufferHandle);

	QFE::FRAMEWORK::DispatchCompute(
		graphicEngine.get(), resources.updateParticlePSOHandle,
		{ resources.particleCountBufferHandle, resources.particlePerFrameBufferHandle,
		  resources.particleBufferHandle },
		{ cbv, cbv, uav }, WindowsEngineResources::kParticleCount);
}

void QFE::FRAMEWORK::EnginePostDraw(WindowsQuickForgeEngineSystems& systems, WindowsEngineResources& resources) {
	auto& gameWindowManager = systems.windowManager;
	auto& graphicEngine = systems.graphicEngine;
	auto& guiManager = systems.guiManager;
	auto& inputInterface = systems.inputInterface;
	auto& fpsCounter = systems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *systems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	QFE::FRAMEWORK::DrawSceneModels(
		sceneManager, graphicEngine.get(), resources.psoHandle, resources.viewportHandle,
		resources.scissorRectHandle, resources.renderTargets, resources.rootParameterTypes);

	std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources(4);
	for (int i = 0; i < 4; ++i) {
		QFE::FRAMEWORK::GetRenderResourceHandle(graphicEngine.get(), resources.renderTargets[i], rayTracingRootResources[i]);
	}

	// カメラの位置をGPUに送るための定数バッファを作成
	QFE::GRAPHIC::DirectXResourceHandle cameraBufferHandle;
	cameraBufferHandle = graphicEngine->GetDirectXResourceAllocator()->AllocateConstantBuffer<CameraForGPU>("CameraBuffer");
	CameraForGPU* cameraPos = graphicEngine->GetConstantBufferData<CameraForGPU>(cameraBufferHandle);
	if(cameraPos == nullptr) {
		assert(false && "Failed to get CameraForGPU constant buffer data.");
		return;
	}
	resources.cameraTransform.translate;
	cameraPos->cameraPosition = 
		QFE::MATH::Vector3(
			resources.cameraTransform.translate.x,
			resources.cameraTransform.translate.y,
			resources.cameraTransform.translate.z);

	QFE::GRAPHIC::DirectXResourceHandle textureFirstResourceHandle;
	QFE::FRAMEWORK::GetBlackCubeMapTextureHandle(graphicEngine.get(), textureFirstResourceHandle);

	QFE::FRAMEWORK::ShadowSpecularRayTracingPSO(
		graphicEngine.get(), resources.rtpsoHandle, resources.uavBufferHandle,
		cameraBufferHandle, resources.globalTriHandle, resources.globalUVHandle,
		resources.instanceMetaHandle, textureFirstResourceHandle, rayTracingRootResources,
		resources.finalRenderTargetHandle);

	QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle;
	if (QFE::FRAMEWORK::GetDepthStencilResourceHandle(graphicEngine.get(), depthStencilHandle)) {
		QFE::FRAMEWORK::SetRenderTarget(graphicEngine.get(), depthStencilHandle, { QFE::GRAPHIC::RenderTargetHandle::SwapChain });
	}
}

bool QFE::FRAMEWORK::LoadModelVertexData(
	WindowsQuickForgeEngineSystems& systems, 
	const std::string& modelDir, const std::string& modelName,
	std::unordered_map<std::string, QFE::ASSET::ModelData>& modelData,
	std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle>& vertexBuffers) {

	QFE::ASSET::AssimpModelLoader* modelLoader = systems.modelLoader.get();

	if (modelLoader->LoadModel(modelDir + modelName + ".obj", modelData[modelName])) {
		bool result = QFE::FRAMEWORK::CreateVertexBuffer(
			systems.graphicEngine.get(),
			modelData[modelName].meshes[0].vertices.GetInternalVector(),
			modelName, vertexBuffers[modelName]);
		return result;
	}
	return false;
}

bool QFE::FRAMEWORK::LoadModelIndexBuffer(
	WindowsQuickForgeEngineSystems& systems, 
	const std::string& modelDir, const std::string& modelName,
	std::unordered_map<std::string, QFE::ASSET::ModelData>& modelData,
	std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle>& indexBuffers) {

	QFE::ASSET::AssimpModelLoader* modelLoader = systems.modelLoader.get();
	if (modelLoader->LoadModel(modelDir + modelName + ".obj", modelData[modelName])) {
		bool result = QFE::FRAMEWORK::CreateIndexBuffer(
			systems.graphicEngine.get(),
			modelData[modelName].meshes[0].indices.GetInternalVector(),
			modelName, indexBuffers[modelName]);
		return result;
	}
	return false;
}

bool QFE::FRAMEWORK::LoadModelAndCreateBLAS(
	WindowsQuickForgeEngineSystems& systems, 
	const std::string& modelDir, const std::string& modelName,
	std::unordered_map<std::string, QFE::ASSET::ModelData>& modelData,
	std::unordered_map<std::string, QFE::GRAPHIC::BLASHandle>& blasHandles) {

	QFE::ASSET::AssimpModelLoader* modelLoader = systems.modelLoader.get();
	if (modelLoader->LoadModel(modelDir + modelName + ".obj", modelData[modelName])) {
		bool result = QFE::FRAMEWORK::CreateBLAS(
			systems.graphicEngine.get(), modelData[modelName].meshes[0].vertices.GetInternalVector(),
			modelData[modelName].meshes[0].indices.GetInternalVector(),
			modelName, blasHandles[modelName]);
		return result;
	}
	return false;
}

bool QFE::FRAMEWORK::LoadTexture(
	WindowsQuickForgeEngineSystems& systems, 
	const std::string& textureDir, const std::string& textureName, 
	std::map<std::string, QFE::GRAPHIC::DirectXResourceHandle>& textureHandles,
	std::map<std::string, uint32_t>& textureGpuIndexMap, uint32_t& nextTextureGpuIndex) {
	
	// すでに読み込まれている場合はスキップ
	if (textureHandles.find(textureName) != textureHandles.end()) {	
		return true;
	}

	std::string texturePath = textureDir + textureName;
	QFE::GRAPHIC::DirectXResourceHandle textureHandle;
	bool result = QFE::FRAMEWORK::LoadTextureFromFile(systems.graphicEngine.get(), texturePath, textureHandle);
	if (result) {
		textureHandles[textureName] = textureHandle;
		textureGpuIndexMap[textureName] = nextTextureGpuIndex;
		++nextTextureGpuIndex;
	}
	return result;
	return false;
}
