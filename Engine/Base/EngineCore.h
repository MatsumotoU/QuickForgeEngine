#pragma once

#ifdef _DEBUG
#include "MyDebugLog.h"
#endif // _DEBUG

// 現状ないと動かないコアたち
#include <wrl.h>
#include <Windows.h>
#include <d3d12.h>
#include "Windows/WinApp.h"
#include "DirectX/DirectXCommon.h"
#include "DirectX/ImGuiManager.h"
// ディスクリプタヒープ
#include "DirectX/Descriptors/RtvDescriptorHeap.h"
#include "DirectX/Descriptors/SrvDescriptorHeap.h"

// Fps管理
#include "DirectX/FramePerSecond.h"
// 描画機能
#include "DirectX/TextureManager.h"
#include "DirectX/GraphicsCommon.h"
// 音声機能
#include "Audio/XAudioCore.h"
#include "Audio/Audio3D.h"
#include "Audio/AudioResourceManager.h"
#include "Audio/AudioSourceBinder.h"
#include "Audio/AudioPlayer.h"

// 入力機能
#include "Input/DirectInput/DirectInputManager.h"
#include "Input/XInput/XInputController.h"

// Resorce
#include "DirectX/MaterialResource.h"
#include "DirectX/WVPResource.h"
#include "DirectX/DirectionalLightResource.h"
// Math
#include "Math/Matrix/Matrix3x3.h"
#include "Math/Transform.h"
#include "Math/VerTexData.h"
#include "Math/TransformationMatrix.h"
#include "Utility/MyGameMath.h"
#include "Utility/MyEasing.h"
// Object
#include "Object/DirectionalLight.h"
#include "Object/Material.h"
// Debug
#include "Base/MyString.h"
#include "Sprite/Sprite.h"
// Audio
#include "Audio/AudioEmitter.h"
#include "Audio/AudioListener.h"
// Camera
#include "Camera/DebugCamera.h"
#include "Camera/Camera.h"
// Model
#include "Model/Model.h"
#include "Model/Billboard.h"

// Json
#include "../Utility/SimpleJson.h"

// ファイル操作
#include "../Utility/FileLoader.h"

// String
#include "String/StringLiblary.h"

// GraphRenderer
#include "DirectX/GraphRenderer.h"

// TODO: ビルドツールpremakeの導入 

class EngineCore final {
public:
	EngineCore();
	~EngineCore();

public:
	void Initialize(LPCWSTR windowName, HINSTANCE hInstance, LPSTR lpCmdLine,MSG* msg);
	void Update();

public:
	void PreDraw();
	void PostDraw();

public:// エンジンに包含されている機能を間接的に使う関数
	/// <summary>
	/// ゲームで使用する音声データをゲームに使える状態にします
	/// </summary>
	/// <param name="filePath"></param>
	/// <param name="fileName"></param>
	/// <returns>音声データの管理番号</returns>
	uint32_t LoadSoundData(const std::string& filePath, const std::string& fileName);

public:// エンジンの機能取得
	WinApp* GetWinApp();
	DirectXCommon* GetDirectXCommon();
	ImGuiManager* GetImGuiManager();
	TextureManager* GetTextureManager();
	GraphicsCommon* GetGraphicsCommon();
	XAudioCore* GetAudioManager();
	Audio3D* GetAudio3D();
	DirectInputManager* GetInputManager();
	Sprite* GetOffscreen();
	FramePerSecond* GetFpsCounter();
	RtvDescriptorHeap* GetRtvDescriptorHeap();
	SrvDescriptorHeap* GetSrvDescriptorHeap();
	GraphRenderer* GetGraphRenderer();
	XInputController* GetXInputController();
	AudioResourceManager* GetAudioResourceManager();
	AudioSourceBinder* GetAudioSourceBinder();
	AudioPlayer* GetAudioPlayer();

public:
	 float GetDeltaTime();

private:// コア機能
	WinApp winApp_;
	MSG* msg_;
	DirectXCommon dxCommon_;
	ImGuiManager imGuiManager_;

private:// 時間管理
	FramePerSecond fpsCounter_;
	float deltaTime_;

private:// 描画機能
	TextureManager textureManager_;
	GraphicsCommon graphicsCommon_;
	GraphRenderer graphRenderer_;

private:// 音声
	XAudioCore audioManager_;
	Audio3D audio3D_;
	AudioResourceManager audioResourceManager_;
	AudioSourceBinder audioSourceBinder_;
	AudioPlayer audioPlayer_;

private:// 入力
	DirectInputManager inputManager_;
	XInputController xController_;
	
private:// ディスクリプタヒープ
	RtvDescriptorHeap rtvDescriptorHeap_;
	SrvDescriptorHeap srvDescriptorHeap_;

private:// デバッグ用一時変数
	Sprite offscreen_;
	Transform offscreenTransform_;
	Transform uvTransform_;
	Camera camera_;
};