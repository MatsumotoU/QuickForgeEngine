// 現状ないと動かないコアたち
#include "Engine/Base/Windows/WinApp.h"
#include "Engine/Base/DirectX/DirectXCommon.h"
#include "Engine/Base/DirectX/shaderCompiler.h"
#include "Engine/Base/DirectX/ImGuiManager.h"
#include "Engine/Base/DirectX/TextureManager.h"
#include "Engine/Base/DirectX/ModelManager.h"
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
#include "Engine/Audio/MultiAudioLoader.h"

// Input
#include "Engine/Input/DirectInput/DirectInputManager.h"

// Camera
#include "Engine/Camera/Camera.h"
#include "Engine/Camera/DebugCamera.h"

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

	MultiAudioLoader multiAudioLoader;
	multiAudioLoader.Initialize();

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
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	int32_t ballTextureIndex = 0;

	Sprite sprite(&dxCommon, &textureManager, &imGuiManager, 640, 360,&pso);

	SoundData soundData1 = Audiomanager::SoundLoadWave("Resources/Alarm01.wav");
	SoundData soundData2 = Audiomanager::SoundLoadMp3("Resources/Enter.mp3");

	Camera camera;
	camera.Initialize(&winApp);

	DebugCamera debugCamera;
	debugCamera.Initialize(&winApp, &input);

	// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	WVPResource wvp;
	wvp.Initialize(&dxCommon);
	//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(dxCommon.GetDevice(), sizeof(TransformationMatrix));
	//// データを書き込む
	//TransformationMatrix* wvpData = nullptr;
	//// 書き込むためのアドレスを取得
	//wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//// 単位行列を書き込んでおく
	//wvpData->WVP = Matrix4x4::MakeIndentity4x4();
	//wvpData->World = Matrix4x4::MakeIndentity4x4();

	// マテリアル用のリソースを作る
	MaterialResource material;
	material.Initialize(&dxCommon);
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(dxCommon.GetDevice(), sizeof(Material));
	//// マテリアルにデータを書き込む
	//Material* materialData = nullptr;
	//// 書き込むためのアドレス取得
	//materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//// 今回は赤を書き込んでみる
	//materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialData->enableLighting = true;
	//materialData->uvTransform = Matrix4x4::MakeIndentity4x4();

	// 光源用のリソースを作る。DirectionnalLight一つ分のサイズを用意する
	DirectionalLightResource directionalLight;
	directionalLight.Initialize(&dxCommon);
	//Microsoft::WRL::ComPtr<ID3D12Resource> directionnalLightResource = CreateBufferResource(dxCommon.GetDevice(), sizeof(DirectionalLight));
	//// データを書き込む
	//DirectionalLight* directionnalLightData = nullptr;
	//// 書き込むためのアドレスを取得
	//directionnalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionnalLightData));
	//// 情報を書き込んでおく
	//directionnalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	//directionnalLightData->direction = { 0.0f,-1.0f,0.0f };
	//directionnalLightData->intensity = 1.0f;

	/*vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f};
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	vertexData[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	vertexData[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	vertexData[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };*/

	//// Spriteを作る
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * 4);
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	//vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//VertexData* vertexDataSprite = nullptr;
	//vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
	//vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	//vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	//vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	//vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	//vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	//vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };
	//vertexDataSprite[3].texcoord = { 1.0f,0.0f };

	//// マテリアル用のリソースを作る
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	//// マテリアルにデータを書き込む
	//Material* materialDataSprite = nullptr;
	//// 書き込むためのアドレス取得
	//materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//// 今回は赤を書き込んでみる
	//materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialDataSprite->enableLighting = false;
	//materialDataSprite->uvTransform = Matrix4x4::MakeIndentity4x4();

	//// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));
	//// データを書き込む
	//TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//// 書き込むためのアドレスを取得
	//transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//// 単位行列を書き込んでおく
	//transformationMatrixDataSprite->WVP = Matrix4x4::MakeIndentity4x4();
	//transformationMatrixDataSprite->World = Matrix4x4::MakeIndentity4x4();

	//// indexBuffer
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(uint32_t) * 6);
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	//uint32_t* indexDataSprite = nullptr;
	//indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	//indexDataSprite[0] = 0;
	//indexDataSprite[1] = 1;
	//indexDataSprite[2] = 2;
	//indexDataSprite[3] = 1;
	//indexDataSprite[4] = 3;
	//indexDataSprite[5] = 2;

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

	// ModelDataを使う
	ModelData modelData = ModelManager::LoadObjFile("Resources", "plane.obj");
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(dxCommon.GetDevice(), sizeof(VertexData) * modelData.vertices.size());
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// テクスチャを読み込む
	//int32_t monsterBallHandle = textureManager.LoadTexture("Resources/monsterBall.png");
	int32_t modelHandle = textureManager.LoadTexture(modelData.material.textureFilePath);

	// * ビューポートとシザー * //
	ViewPort viewport;
	viewport.Inititalize(&winApp);

	ScissorRect scissor;
	scissor.Initialize(&winApp);

	bool Lighting = true;
	Vector4 color{1.0f,1.0f,1.0f,1.0f};

	float vol = 1.0f;
	float pit = 1.0f;

	// ウィンドウのXボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// windowにメッセージが基底たら最優先で処理される
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			input.Update();
			debugCamera.Update();

			//transform.rotate.y += 0.01f;
			material.materialData_->color = color;
			sprite.material_.materialData_->color = color;

			Matrix4x4 uvTransformMatrix = Matrix4x4::MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeTranslateMatrix(uvTransformSprite.translate));
			material.materialData_->uvTransform = uvTransformMatrix;

			Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 worldMatrixSprite = Matrix4x4::MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 worldViewProjectionMatrix = debugCamera.camera_.MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_PERSPECTIVE);
			Matrix4x4 worldViewProjectionMatrixSprite = debugCamera.camera_.MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_ORTHOGRAPHIC);

			wvp.wvpData_->WVP = worldViewProjectionMatrix;
			wvp.wvpData_->World = worldMatrix;

			dxCommon.PreDraw();
			imGuiManager.BeginFrame();

			debugCamera.DrawImGui();

			ImGui::ColorPicker4("color", &color.x);

			// テクスチャをvramにアップロード
			textureManager.PreDraw();

			ID3D12GraphicsCommandList* commandList = dxCommon.GetCommandList();

			// Depthの設定
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = pso.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
			commandList->OMSetRenderTargets(1, dxCommon.GetRtvHandles(), false, &dsvHandl);
			commandList->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			// 球体
			commandList->RSSetViewports(1, viewport.GetViewport());
			commandList->RSSetScissorRects(1, scissor.GetScissorRect());
			commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			commandList->SetPipelineState(pso.GetPipelineState());
			//commandList->IASetIndexBuffer(&indexBufferView);
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->SetGraphicsRootConstantBufferView(0, material.GetMaterial()->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(1, wvp.GetWVPResource()->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetDirectionalLightResource()->GetGPUVirtualAddress());
			
			commandList->SetGraphicsRootDescriptorTable(2, textureManager.GetTextureSrvHandleGPU(modelHandle));
			commandList->DrawInstanced(static_cast<UINT>(modelData.vertices.size()), 1, 0, 0);
			//commandList->DrawIndexedInstanced(kSubdivision* kSubdivision * 6, 1, 0, 0, 0);// 描画処理をする頂点の数

			// sprite
			//sprite.DrawSprite(monsterBallHandle,&viewport,&scissor);
			//Matrix4x4 uvTransformMatrix = Matrix4x4::MakeScaleMatrix(uvTransformSprite.scale);
			//uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeRotateZMatrix(uvTransformSprite.rotate.z));
			//uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeTranslateMatrix(uvTransformSprite.translate));
			//materialDataSprite->uvTransform = uvTransformMatrix;

			//commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
			//commandList->IASetIndexBuffer(&indexBufferViewSprite);
			//commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			//commandList->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureSrvHandleGPU(0));
			//
			//commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			//commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

			ImGui::InputFloat2("mouseDir", &input.mouse_.mouseMoveDir_.x);
			ImGui::InputFloat3("debugCameraTranslate", &debugCamera.camera_.transform_.translate.x);

			/*if (input.mouse_.GetPress(0)) {
				ImGui::ShowDemoWindow();
			}*/

			ImGui::InputFloat("Volume", &vol);
			ImGui::InputFloat("Pitch", &pit);

			if (ImGui::Button("SE1Start")) {
				Audiomanager::SoundPlayWave(audioManager.xAudio2_.Get(), soundData1,vol,pit);
			}
			if (ImGui::Button("SE2Start")) {
				Audiomanager::SoundPlayWave(audioManager.xAudio2_.Get(), soundData2, vol, pit);
			}
			ImGui::InputInt("textureIndex", &ballTextureIndex, 1);
			if (ballTextureIndex < 0) {
				ballTextureIndex = 0;
			}
			if (ballTextureIndex > 1) {
				ballTextureIndex = 1;
			}
			ImGui::DragFloat3("cametaTransition", &camera.transform_.translate.x,0.1f);
			ImGui::DragFloat3("cametaRotate", &camera.transform_.rotate.x, 0.1f);
			ImGui::DragFloat3("ObjRotate", &transform.rotate.x, 0.1f);
			ImGui::DragFloat3("directionnalLight.direction", &directionalLight.directionalLightData_->direction.x, 0.1f);
			ImGui::Checkbox("isLighting", &Lighting);
			material.materialData_->enableLighting = Lighting;
			directionalLight.directionalLightData_->direction = directionalLight.directionalLightData_->direction.Normalize();

			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f,-10.0f,10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

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