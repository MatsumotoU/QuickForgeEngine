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

	// Modelを使う
	Model model;
	model.Initialize(&dxCommon, &textureManager, &pso);
	model.LoadModel("Resources", "skyDome.obj", COORDINATESYSTEM_HAND_RIGHT);

	// テクスチャを読み込む
	//int32_t monsterBallHandle = textureManager.LoadTexture("Resources/monsterBall.png");

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
			//transform.rotate.y += 0.05f;
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

			// sprite
			//sprite.DrawSprite(transform2,monsterBallHandle,&debugCamera.camera_);

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