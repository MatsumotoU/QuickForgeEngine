#pragma once

// 現状ないと動かないコアたち
#include <wrl.h>
#include <Windows.h>
#include <d3d12.h>
#include "Windows/WinApp.h"
#include "DirectX/DirectXCommon.h"
#include "DirectX/ImGuiManager.h"
// 描画機能
#include "DirectX/TextureManager.h"
#include "DirectX/GraphicsCommon.h"
// 音声機能
#include "../Audio/AudioManager.h"
#include "../Audio/Audio3D.h"
// 入力機能
#include "../Input/DirectInput/DirectInputManager.h"

class EngineCore {
public:
	EngineCore();
	~EngineCore();

public:
	void Initialize(LPCWSTR windowName, HINSTANCE hInstance, LPSTR lpCmdLine,MSG* msg);
	void Update();

public:
	void PreDraw();
	void PostDraw();

public:
	WinApp* GetWinApp();
	DirectXCommon* GetDirectXCommon();
	ImGuiManager* GetImGuiManager();
	TextureManager* GetTextureManager();
	GraphicsCommon* GetGraphicsCommon();
	AudioManager* GetAudioManager();
	Audio3D* GetAudio3D();
	DirectInputManager* GetInputManager();

private:// コア機能
	WinApp winApp_;
	MSG* msg_;
	DirectXCommon dxCommon_;
	ImGuiManager imGuiManager_;

private:// 描画機能
	TextureManager textureManager_;
	GraphicsCommon graphicsCommon_;

private:// 音声
	AudioManager audioManager_;
	Audio3D audio3D_;

private:// 入力
	DirectInputManager inputManager_;
	
private:// ディスクリプタヒープ
	/*Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dvrDescriptorHeap_;*/
};