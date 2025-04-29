// 現状ないと動かないコアたち
#include "Engine/Base/Windows/WinApp.h"
#include "Engine/Base/DirectX/DirectXCommon.h"
#include "Engine/Base/DirectX/shaderCompiler.h"
#include "Engine/Base/DirectX/ImGuiManager.h"
#include "Engine/Base/DirectX/TextureManager.h"
#include "Engine/Model/ModelManager.h"
// PSO
#include "Engine/Base/DirectX/PipelineStateObject.h"
// シザーとビューポート
#include "Engine/Base/DirectX/Viewport.h"
#include "Engine/Base/DirectX/ScissorRect.h"

// Resorce
#include "Engine/Base/DirectX/MaterialResource.h"
#include "Engine/Base/DirectX/WVPResource.h"
#include "Engine/Base/DirectX/DirectionalLightResource.h"

// Math
#include "Engine/Math/Matrix/Matrix3x3.h"
#include "Engine/Math/Transform.h"
#include "Engine/Math/VerTexData.h"
#include "Engine/Math/TransformationMatrix.h"

// Object
#include "Engine/Object/DirectionalLight.h"
#include "Engine/Object/Material.h"

// Debug
#include "Engine/Base/Windows/WinAppDebugCore.h"
#include "Engine/Base/DirectX/DirectX12DebugCore.h"
#include "Engine/Base/MyString.h"

#include "Engine/Sprite/Sprite.h"

// Audio
#include "Engine/Audio/AudioManager.h"

// Input
#include "Engine/Input/DirectInput/DirectInputManager.h"

// Camera
#include "Engine/Camera/DebugCamera.h"

// Model
#include "Engine/Model/Model.h"

//srv,rtv,dsvはimGuiManager,dxCommon,textureManagerが持っているので要分離(こいつらは増えない)
// miniEngine Cocos2D Ogre3D ら辺が参考

//miniEngineのdevice管理は安全なのか？(extern)

// windowsアプリでのエントリーポイント(main関数) 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int) {
	
	// * ゲーム以前の設定 * //
	WinAppDebugCore winAppDbgCore(lpCmdLine);
	DirectX12DebugCore directXDbgCore;

	// ウィンドウ生成
	WinApp winApp;
	winApp.CreateGameWindow();
	MSG msg{};

	// DirectXの初期化																				
	DirectXCommon dxCommon;
	dxCommon.SetCommandLine(&lpCmdLine);
	dxCommon.Initialize(&winApp);

	//se
	AudioManager audioManager;
	audioManager.Initialize();

	// Input
	DirectInputManager input;
	input.Initialize(&winApp, hInstance);

	// ImGuiの初期化
	ImGuiManager imGuiManager;
	imGuiManager.Initialize(&winApp,&dxCommon);

	// TextuerManagerの初期化
	TextureManager textureManager;
	textureManager.Initialize(&dxCommon,&imGuiManager);

	// * PSOを作成 * //
	PipelineStateObject pso;
	pso.Initialize(&dxCommon,&winApp);

	// * ゲーム内の設定 * //
	// 変数定義
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform transform2{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	Sprite sprite(&dxCommon, &textureManager, &imGuiManager, 640.0f, 320.0f,&pso);

	SoundData soundData1 = Audiomanager::SoundLoadWave("Resources/Alarm01.wav");
	SoundData soundData2 = Audiomanager::SoundLoadMp3("Resources/Enter.mp3");

	Camera camera;
	camera.Initialize(&winApp);

	DebugCamera debugCamera;
	debugCamera.Initialize(&winApp, &input);

	// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	WVPResource wvp;
	wvp.Initialize(&dxCommon);

	// マテリアル用のリソースを作る
	MaterialResource material;
	material.Initialize(&dxCommon);

	// 光源用のリソースを作る。DirectionnalLight一つ分のサイズを用意する
	DirectionalLightResource directionalLight;
	directionalLight.Initialize(&dxCommon);

	//// * VertexResourceを生成する * //
	//const int32_t kSubdivision = 16;
	//const int32_t sphereVertexMax = kSubdivision * kSubdivision * 4;
	//ID3D12Resource* vertexResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * sphereVertexMax);

	//// * VertexBufferViewを作成する * //
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//vertexBufferView.SizeInBytes = sizeof(VertexData) * sphereVertexMax;
	//vertexBufferView.StrideInBytes = sizeof(VertexData);

	//// * リソースにデータを書き込む * //
	//VertexData* vertexData = nullptr;
	//vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//// indexBuffer
	//ID3D12Resource* indexResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(uint32_t) * kSubdivision * kSubdivision * 6);
	//D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	//indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//indexBufferView.SizeInBytes = sizeof(uint32_t) * kSubdivision * kSubdivision * 6;
	//indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	//uint32_t* indexData = nullptr;
	//indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	//// 球
	//const float pi = DirectX::XM_PI;
	//
	//const float kLonEvery = pi * 2.0f / static_cast<float>(kSubdivision);
	//const float kLatEvery = pi / static_cast<float>(kSubdivision);
	//for (int32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	float lat = -pi / 2.0f + kLatEvery * static_cast<float>(latIndex);
	//	for (int32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		uint32_t start = (latIndex * kSubdivision + lonIndex) * 4;
	//		uint32_t startIndex = (latIndex * kSubdivision + lonIndex) * 6;
	//		float lon = static_cast<float>(lonIndex) * kLonEvery;

	//		Vector4 a = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon),1.0f };
	//		Vector4 b = { cos(lat + kLatEvery) * cos(lon),sin(lat+ kLatEvery),cos(lat+ kLatEvery) * sin(lon),1.0f };
	//		Vector4 c = { cos(lat) * cos(lon+ kLonEvery),sin(lat),cos(lat) * sin(lon+ kLonEvery),1.0f };
	//		Vector4 d = { cos(lat + kLatEvery) * cos(lon + kLonEvery),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon + kLonEvery),1.0f };

	//		Vector3 aNormal = { a.x,a.y,a.z };
	//		Vector3 bNormal = {	b.x,b.y,b.z };
	//		Vector3 cNormal = { c.x,c.y,c.z };
	//		Vector3 dNormal = { d.x,d.y,d.z };

	//		vertexData[start].position = a;
	//		vertexData[start].texcoord = { 
	//			static_cast<float>(lonIndex) / static_cast<float>(kSubdivision),
	//			1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision) };
	//		vertexData[start].normal = aNormal;
	//		vertexData[start + 1].position = b;
	//		vertexData[start + 1].texcoord = { 
	//			static_cast<float>(lonIndex) / static_cast<float>(kSubdivision),
	//			1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision) };
	//		vertexData[start+1].normal = bNormal;
	//		vertexData[start + 2].position = c;
	//		vertexData[start + 2].texcoord = { 
	//			static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision),
	//			1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision) };
	//		vertexData[start+2].normal = cNormal;
	//		vertexData[start + 3].position = d;
	//		vertexData[start + 3].texcoord = {
	//			static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision),
	//			1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision) };
	//		vertexData[start + 3].normal = dNormal;

	//		indexData[startIndex] = start;
	//		indexData[startIndex+1] = start+1;
	//		indexData[startIndex+2] = start+2;
	//		indexData[startIndex+3] = start+1;
	//		indexData[startIndex+4] = start+3;
	//		indexData[startIndex+5] = start+2;

	//		/*vertexData[start + 4].position = d;
	//		vertexData[start + 4].texcoord = {
	//			static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision),
	//			1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision) };
	//		vertexData[start + 4].normal = dNormal;
	//		vertexData[start + 5].position = c;
	//		vertexData[start + 5].texcoord = {
	//			static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision),
	//			1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision) };
	//		vertexData[start + 5].normal = cNormal;*/
	//	}
	//}

	// Modelを使う
	Model model;
	model.Initialize(&dxCommon, &textureManager, &pso);
	model.LoadModel("Resources", "plane.obj");

	Model model2;
	model2.Initialize(&dxCommon, &textureManager, &pso);
	model2.LoadModel("Resources", "plane.obj");

	// ModelDataを使う
	/*ModelData modelData = Modelmanager::LoadObjFile("Resources", "plane.obj");
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(dxCommon.GetDevice(), sizeof(VertexData) * modelData.vertices.size());
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());*/

	// テクスチャを読み込む
	int32_t monsterBallHandle = textureManager.LoadTexture("Resources/monsterBall.png");

	// * ビューポートとシザー * //
	ViewPort viewport;
	viewport.Inititalize(&winApp);

	ScissorRect scissor;
	scissor.Initialize(&winApp);

	bool Lighting = true;
	Vector4 color{1.0f,1.0f,1.0f,1.0f};

	// ウィンドウのXボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// windowにメッセージが基底たら最優先で処理される
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// Update
			input.Update();
			debugCamera.Update();
			transform.rotate.y += 0.05f;
			sprite.material_.materialData_->color = color;

			// draw
			dxCommon.PreDraw();
			imGuiManager.BeginFrame();
			textureManager.PreDraw();

			debugCamera.DrawImGui();
			

			ID3D12GraphicsCommandList* commandList = dxCommon.GetCommandList();

			// Depthの設定
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = pso.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
			commandList->OMSetRenderTargets(1, dxCommon.GetRtvHandles(), false, &dsvHandl);
			commandList->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			// モデルの描画
			model.Draw(transform, &debugCamera.camera_);
			model2.Draw(transform2, &debugCamera.camera_);

			// sprite
			sprite.DrawSprite(transform2,monsterBallHandle,&debugCamera.camera_);

			// ImGui
			ImGui::ColorPicker4("color", &color.x);
			ImGui::DragFloat3("ObjTranslate", &transform.translate.x, 0.1f);
			ImGui::DragFloat3("ObjRotate", &transform.rotate.x, 0.1f);
			ImGui::DragFloat3("ObjScale", &transform.scale.x, 0.1f);
			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
			ImGui::DragFloat3("directionnalLight.direction", &directionalLight.directionalLightData_->direction.x, 0.1f);
			ImGui::Checkbox("isLighting", &Lighting);
			material.materialData_->enableLighting = Lighting;
			directionalLight.directionalLightData_->direction = directionalLight.directionalLightData_->direction.Normalize();

			// 描画終了処理
			textureManager.PostDraw();
			imGuiManager.EndFrame();
			dxCommon.PostDraw();
			textureManager.ReleaseIntermediateResources();
		}
	}

	Audiomanager::SoundUnload(&soundData1);

	// * 解放処理 * //
	//textureManager.Finalize();
	imGuiManager.EndImGui();
	dxCommon.ReleaseDirectXObject();
	CloseWindow(winApp.GetHWND());
	return 0;
}