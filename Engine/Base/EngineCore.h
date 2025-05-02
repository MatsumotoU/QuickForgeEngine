#pragma once
// 現状ないと動かないコアたち
#include <wrl.h>
#include <Windows.h>
#include <d3d12.h>
class WinApp;
class DirectXCommon;
class ImGuiManager;
class TextureManager;

class EngineCore {
public:
	EngineCore();
	~EngineCore();

public:
	void Initialize(LPCWSTR windowName, HINSTANCE hInstance, LPSTR lpCmdLine,MSG* msg);

public:
	void PreDraw();
	void PostDraw();

public:
	WinApp* GetWinApp();
	DirectXCommon* GetDirectXCommon();
	ImGuiManager* GetImGuiManager();
	TextureManager* GetTextureManager();

private:// コア機能
	WinApp* winApp_;
	MSG* msg_;
	DirectXCommon* dxCommon_;
	ImGuiManager* imGuiManager_;
	TextureManager* textureManager_;
	
private:// ディスクリプタヒープ
	/*Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dvrDescriptorHeap_;*/
};