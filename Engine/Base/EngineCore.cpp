#include "EngineCore.h"
#include "Windows/WinApp.h"
#include "DirectX/DirectXCommon.h"
#include "DirectX/ImGuiManager.h"
#include "DirectX/TextureManager.h"

EngineCore::EngineCore() {
	msg_ = nullptr;
	winApp_ = new WinApp;
	dxCommon_ = new DirectXCommon;
	imGuiManager_ = new ImGuiManager;
	textureManager_ = new TextureManager;
}

EngineCore::~EngineCore() {
	// 機能の終了処理
	textureManager_->Finalize();
	imGuiManager_->EndImGui();
	dxCommon_->ReleaseDirectXObject();
	CloseWindow(winApp_->GetHWND());

	// インスタンス破棄
	delete textureManager_;
	delete imGuiManager_;
	delete dxCommon_;
	delete winApp_;
	// 一応のポインタ削除
	textureManager_ = nullptr;
	imGuiManager_ = nullptr;
	dxCommon_ = nullptr;
	winApp_ = nullptr;
}

void EngineCore::Initialize(LPCWSTR windowName, HINSTANCE hInstance, LPSTR lpCmdLine, MSG* msg) {
	// windowsアプリ管理クラス初期化
	winApp_->CreateGameWindow(windowName);
	msg_ = msg;
	winApp_->SetMSG(msg);
	hInstance;

	// DirectXの初期化
	dxCommon_->SetCommandLine(&lpCmdLine);
	dxCommon_->Initialize(winApp_);

	// imGuiManager初期化
	imGuiManager_->Initialize(winApp_,dxCommon_);

	// テクスチャマネージャの初期化
	textureManager_->Initialize(dxCommon_, imGuiManager_);
}

void EngineCore::PreDraw() {
	dxCommon_->PreDraw();
	imGuiManager_->BeginFrame();
	textureManager_->PreDraw();
}

void EngineCore::PostDraw() {
	textureManager_->PostDraw();
	imGuiManager_->EndFrame();
	dxCommon_->PostDraw();
	textureManager_->ReleaseIntermediateResources();
}

WinApp* EngineCore::GetWinApp() {
	return winApp_;
}

DirectXCommon* EngineCore::GetDirectXCommon() {
	return dxCommon_;
}

ImGuiManager* EngineCore::GetImGuiManager() {
	return imGuiManager_;
}

TextureManager* EngineCore::GetTextureManager() {
	return textureManager_;
}
