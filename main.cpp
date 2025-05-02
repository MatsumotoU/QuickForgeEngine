// 現状ないと動かないコアたち
#include "Engine/Base/EngineCore.h"
#include "Engine/Base/Windows/WinApp.h"
#include "Engine/Base/DirectX/DirectXCommon.h"
#include "Engine/Base/DirectX/shaderCompiler.h"
#include "Engine/Base/DirectX/ImGuiManager.h"
#include "Engine/Base/DirectX/TextureManager.h"
#include "Engine/Model/ModelManager.h"
// PSO
#include "Engine/Base/DirectX/PipelineStateObject.h"

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

	// エンジンの初期化
	MSG msg{};
	EngineCore engineCore;
	engineCore.Initialize(L"CG2", hInstance, lpCmdLine,&msg);

	// 一部機能を外側に
	WinApp* winApp = engineCore.GetWinApp();
	DirectXCommon* dxCommon = engineCore.GetDirectXCommon();
	ImGuiManager* imGuiManager = engineCore.GetImGuiManager();
	TextureManager* textureManager = engineCore.GetTextureManager();

	//se
	AudioManager audioManager;
	audioManager.Initialize();

	// Input
	DirectInputManager input;
	input.Initialize(winApp, hInstance);

	// * PSOを作成 * //
	PipelineStateObject pso;
	pso.Initialize(dxCommon,winApp);

	// * ゲーム内の設定 * //
	// 変数定義
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform transform2{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	Sprite sprite(dxCommon, textureManager, imGuiManager, 640.0f, 320.0f,&pso);

	SoundData soundData1 = Audiomanager::SoundLoadWave("Resources/Alarm01.wav");
	SoundData soundData2 = Audiomanager::SoundLoadMp3("Resources/Enter.mp3");

	// Camera
	DebugCamera debugCamera;
	debugCamera.Initialize(winApp, &input);

	// Modelを使う
	Model model;
	model.Initialize(dxCommon, textureManager, &pso);
	model.LoadModel("Resources", "skyDome.obj", COORDINATESYSTEM_HAND_RIGHT);

	// テクスチャを読み込む
	//int32_t monsterBallHandle = textureManager->LoadTexture("Resources/monsterBall.png");

	bool Lighting = false;
	Vector4 color{1.0f,1.0f,1.0f,1.0f};

	// ウィンドウのXボタンが押されるまでループ
	while (engineCore.GetWinApp()->GetIsWindowQuit()) {
		// windowにメッセージが基底たら最優先で処理される
		if (engineCore.GetWinApp()->GetCanLoop()) {

			// === Update ===
			input.Update();
			debugCamera.Update();
			sprite.material_.materialData_->color = color;

			// === Draw ===
			engineCore.PreDraw();
			
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
			ImGui::Checkbox("isLighting", &Lighting);
			model.material_.materialData_->enableLighting = Lighting;
			debugCamera.DrawImGui();

			// === EndDraw ===
			engineCore.PostDraw();
		}
	}

	Audiomanager::SoundUnload(&soundData1);

	return 0;
}