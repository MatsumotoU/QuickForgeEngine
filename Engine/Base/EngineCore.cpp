#include "EngineCore.h"

EngineCore::EngineCore() {
	engineStartTime = std::chrono::steady_clock::now();
#ifdef _DEBUG
	DebugLog("[[[EngineStarted]]]");
#endif // _DEBUG

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
	// DirectX初期化
	dxCommon_.Initialize(&winApp_);
	// 各ディスクリプタヒープの作成
	rtvDescriptorHeap_.Initialize(dxCommon_.GetDevice(), 3, false);
	srvDescriptorHeap_.Initialize(dxCommon_.GetDevice(), 128, true);
	// DirectXの画面リソースの初期化とRTV登録
	dxCommon_.InitializeScreenResources(rtvDescriptorHeap_.GetRtvDescriptorHeap());

	// imGuiManager初期化
	imGuiManager_.Initialize(&winApp_,&dxCommon_,srvDescriptorHeap_.GetSrvDescriptorHeap());
	// テクスチャマネージャの初期化
	textureManager_.Initialize(&dxCommon_, &srvDescriptorHeap_);
	graphicsCommon_.Initialize(this);
	// fps監視機能初期化
	fpsCounter_.Initialize();
	fpsCounter_.Update();

	// グラフ描画機能の初期化
	graphRenderer_.Initialize(this);

	// 音声機能の初期化
	audioManager_.Initialize();
	audioResourceManager_.Initialize(this);
	audioSourceBinder_.Initialize(this);
	audioPlayer_.Initialize(this);
	chiptune_.Initialize(this);

	// 入力機能の初期化
	inputManager_.Initialize(&winApp_, hInstance);

	// オフスクリーンのスプライト生成
	offscreen_.Initialize(this,static_cast<float>(winApp_.kWindowWidth), static_cast<float>(winApp_.kWindowHeight));
	offscreen_.material_.materialData_->enableLighting = false;

	camera_.Initialize(&winApp_);
}

void EngineCore::Update() {
	imGuiManager_.BeginFrame();
	fpsCounter_.Update();
	inputManager_.Update();
	xController_.Update();
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
	
	textureManager_.PreDraw();

	// 深度の設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = graphicsCommon_.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetOffscreenRtvHandles(), false, &dsvHandl);
	//dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	graphRenderer_.PreDraw();
}

void EngineCore::PostDraw() {

	graphRenderer_.PostDraw();

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

	// 深度の設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = graphicsCommon_.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetOffscreenRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	offscreen_.DrawSprite(offscreenTransform_,uvTransform_,textureManager_.GetOffscreenSrvHandleGPU(),&camera_);

	textureManager_.PostDraw();
	imGuiManager_.EndFrame();
	dxCommon_.PostDraw();
	textureManager_.ReleaseIntermediateResources();
}

uint32_t EngineCore::LoadSoundData(const std::string& filePath, const std::string& fileName) {
	audioResourceManager_.LoadAudioResource(filePath, fileName);
	return audioResourceManager_.GetAudioDataHandle(fileName);
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

XAudioCore* EngineCore::GetAudioManager() {
	return &audioManager_;
}

Audio3D* EngineCore::GetAudio3D() {
	return &audio3D_;
}

DirectInputManager* EngineCore::GetInputManager() {
	return &inputManager_;
}

Sprite* EngineCore::GetOffscreen() {
	return &offscreen_;
}

FramePerSecond* EngineCore::GetFpsCounter() {
	return &fpsCounter_;
}

RtvDescriptorHeap* EngineCore::GetRtvDescriptorHeap() {
	return &rtvDescriptorHeap_;
}

SrvDescriptorHeap* EngineCore::GetSrvDescriptorHeap() {
	return &srvDescriptorHeap_;
}

GraphRenderer* EngineCore::GetGraphRenderer() {
	return &graphRenderer_;
}

XInputController* EngineCore::GetXInputController() {
	return &xController_;
}

AudioResourceManager* EngineCore::GetAudioResourceManager() {
	return &audioResourceManager_;
}

AudioSourceBinder* EngineCore::GetAudioSourceBinder() {
	return &audioSourceBinder_;
}

AudioPlayer* EngineCore::GetAudioPlayer() {
	return &audioPlayer_;
}

Chiptune* EngineCore::GetChiptune() {
	return &chiptune_;
}

float EngineCore::GetDeltaTime() {
	// FPSが0でない場合はFPSを基にデルタタイムを返す
	if (fpsCounter_.GetFps() != 0.0f) {

		// FPSが変動している場合は60FPS想定でデルタタイムを返す
		if (fabsf(fpsCounter_.GetAverageFps() - fpsCounter_.GetFps()) >= 5.0f) {
			return 1.0f / 60.0f;
		}
		// FPSが安定している場合はそのFPSを基にデルタタイムを返す
		return 1.0f / fpsCounter_.GetFps();
	}
	return 0.0f;
}
