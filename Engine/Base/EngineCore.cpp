#include "EngineCore.h"

EngineCore::EngineCore() {
}

EngineCore::~EngineCore() {
	// 機能の終了処理
	textureManager_.Finalize();
	imGuiManager_.EndImGui();
	dxCommon_.ReleaseDirectXObject();
	CloseWindow(winApp_.GetHWND());
}

void EngineCore::Initialize(LPCWSTR windowName, HINSTANCE hInstance, LPSTR lpCmdLine, MSG* msg) {
	lpCmdLine;
	// windowsアプリ管理クラス初期化
	winApp_.CreateGameWindow(windowName);
	msg_ = msg;
	winApp_.SetMSG(msg);
	hInstance;
	// DirectXの初期化
#ifdef _DEBUG
	dxCommon_.SetCommandLine(&lpCmdLine);
#endif // _DEBUG
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

	// オフスクリーンのスプライト生成
	offscreen_.Initialize(
		&dxCommon_, &textureManager_, &imGuiManager_,
		static_cast<float>(winApp_.kWindowWidth), static_cast<float>(winApp_.kWindowHeight), 
		graphicsCommon_.GetTrianglePso());
}

void EngineCore::Update() {
	inputManager_.Update();
}

void EngineCore::PreDraw() {
	// オフスクリーンのバリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = dxCommon_.GetOffscreenResource();
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// バリア張る
	dxCommon_.GetCommandList()->ResourceBarrier(1, &barrier);

	dxCommon_.PreDraw();
	imGuiManager_.BeginFrame();
	textureManager_.PreDraw();

	// 深度の設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = graphicsCommon_.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//commandList->OMSetRenderTargets(1, dxCommon_->GetOffscreenRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void EngineCore::PostDraw() {
	// オフスクリーンのバリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = dxCommon_.GetOffscreenResource();
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	// バリア張る
	dxCommon_.GetCommandList()->ResourceBarrier(1, &barrier);

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
