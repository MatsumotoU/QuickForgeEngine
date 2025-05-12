#include "EngineCore.h"

//#include "DirectX/DirectXCommon.h"
//#include "DirectX/ImGuiManager.h"
//
//#include "DirectX/TextureManager.h"
//#include "DirectX/GraphicsCommon.h"
//
//#include "../Audio/AudioManager.h"
//#include "../Audio/Audio3D.h"
//
//#include "../Input/DirectInput/DirectInputManager.h"

EngineCore::EngineCore() {
	//// コア機能
	//msg_ = nullptr;
	//winApp_ = new WinApp;
	//dxCommon_ = new DirectXCommon;
	//imGuiManager_ = new ImGuiManager;

	//// 描画機能
	//graphicsCommon_ = new GraphicsCommon;
	//textureManager_ = new TextureManager;

	//// 音声機能
	//audioManager_ = new AudioManager;
	//audio3D_ = audioManager_->GetAudio3D();

	//// 入力機能
	//inputManager_ = new DirectInputManager;
}

EngineCore::~EngineCore() {
	// 機能の終了処理
	textureManager_.Finalize();
	imGuiManager_.EndImGui();
	dxCommon_.ReleaseDirectXObject();
	CloseWindow(winApp_.GetHWND());

	//// インスタンス破棄
	//delete audio3D_;
	//delete audioManager_;
	//delete graphicsCommon_;
	//delete textureManager_;
	//delete imGuiManager_;
	//delete dxCommon_;
	//delete winApp_;
	//// 一応のポインタ削除
	//audio3D_ = nullptr;
	//audioManager_ = nullptr;
	//graphicsCommon_ = nullptr;
	//textureManager_ = nullptr;
	//imGuiManager_ = nullptr;
	//dxCommon_ = nullptr;
	//winApp_ = nullptr;
}

void EngineCore::Initialize(LPCWSTR windowName, HINSTANCE hInstance, LPSTR lpCmdLine, MSG* msg) {
	// windowsアプリ管理クラス初期化
	winApp_.CreateGameWindow(windowName);
	msg_ = msg;
	winApp_.SetMSG(msg);
	hInstance;
	// DirectXの初期化
	dxCommon_.SetCommandLine(&lpCmdLine);
	dxCommon_.Initialize(&winApp_);
	// imGuiManager初期化
	imGuiManager_.Initialize(&winApp_,&dxCommon_);
	// テクスチャマネージャの初期化
	textureManager_.Initialize(&dxCommon_, &imGuiManager_);
	graphicsCommon_.Initialize(&dxCommon_, &winApp_);

	// 音声機能の初期化
	audioManager_.Initialize();

	// 入力機能の初期化
	inputManager_.Initialize(&winApp_, hInstance);
}

void EngineCore::Update() {
	inputManager_.Update();
}

void EngineCore::PreDraw() {
	dxCommon_.PreDraw();
	imGuiManager_.BeginFrame();
	textureManager_.PreDraw();
}

void EngineCore::PostDraw() {
	textureManager_.PostDraw();
	imGuiManager_.EndFrame();
	dxCommon_.PostDraw();
	textureManager_.ReleaseIntermediateResources();
}

WinApp* EngineCore::GetWinApp() {
	return &winApp_;
}

DirectXCommon* EngineCore::GetDirectXCommon() {
	return &dxCommon_;
}

ImGuiManager* EngineCore::GetImGuiManager() {
	return &imGuiManager_;
}

TextureManager* EngineCore::GetTextureManager() {
	return &textureManager_;
}

GraphicsCommon* EngineCore::GetGraphicsCommon() {
	return &graphicsCommon_;
}

AudioManager* EngineCore::GetAudioManager() {
	return &audioManager_;
}

Audio3D* EngineCore::GetAudio3D() {
	return &audio3D_;
}

DirectInputManager* EngineCore::GetInputManager() {
	return &inputManager_;
}
