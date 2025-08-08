#pragma once
#define NOMINMAX

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
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
// メモリ管理
#include "Windows/MemoryWatcher.h"
// Fps管理
#include "DirectX/FramePerSecond.h"
// 描画機能
#include "DirectX/TextureManager.h"
#include "DirectX/GraphicsCommon.h"
#include "DirectX/RendaringPostprocess.h"
// 音声機能
#include "Audio/XAudioCore.h"
#include "Audio/Audio3D.h"
#include "Audio/AudioResourceManager.h"
#include "Audio/AudioSourceBinder.h"
#include "Audio/AudioPlayer.h"
#include "Audio/Chiptune.h"
#include "Audio/MusicalScale.h"
// リソース管理
#include "Utility/DirectoryManager.h"

// バッファー
#include "DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "DirectX/Resource/ShaderBuffers/StructuredBuffer.h"
#include "DirectX/Resource/ShaderBuffers/VertexBuffer.h"

// 入力機能
#include "Input/DirectInput/DirectInputManager.h"
#include "Input/XInput/XInputController.h"

// Math
#include "Math/Matrix/Matrix3x3.h"
#include "Math/Transform.h"
#include "Shaders/StructsForGpu/VerTexData.h"
#include "Shaders/StructsForGpu/TransformationMatrix.h"
#include "Utility/MyGameMath.h"
#include "Utility/MyEasing.h"

// Object
#include "Shaders/StructsForGpu/DirectionalLight.h"
#include "Shaders/StructsForGpu/Material.h"
// Debug
#include "Utility/MyString.h"
#include "Sprite/Sprite.h"
// Audio
#include "Audio/AudioEmitter.h"
#include "Audio/AudioListener.h"
// Camera
#include "Camera/DebugCamera.h"
#include "Camera/Camera.h"
// Model
//#include "Model/Model.h"
//#include "Model/Billboard.h"

// 時間管理
#include "Utility/MyTimer.h"
#include "LoopStoper.h"

// Json
#include "../Utility/SimpleJson.h"

// ファイル操作
#include "../Utility/FileLoader.h"
#include "Utility/FileExtension.h"

// String
#include "Utility/StringLiblary.h"
#include "Font/FontDrawer.h"

// GraphRenderer
#include "DirectX/GraphRenderer.h"

// Scene
#include "Scene/SceneManager.h"

//#include "Script/Lua/LuaScriptManager.h"
//#include "Script/Lua/LuaCallFiles.h"
// TODO: ビルドツールpremakeの導入 

#include <chrono>
static inline std::chrono::steady_clock::time_point engineStartTime;

// TODO: ファイル整理-主にBaseGameObjectをまとめる

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
	FramePerSecond* GetFpsCounter();
	RtvDescriptorHeap* GetRtvDescriptorHeap();
	SrvDescriptorHeap* GetSrvDescriptorHeap();
	GraphRenderer* GetGraphRenderer();
	XInputController* GetXInputController();
	AudioResourceManager* GetAudioResourceManager();
	AudioSourceBinder* GetAudioSourceBinder();
	AudioPlayer* GetAudioPlayer();
	Chiptune* GetChiptune();
	ShaderCompiler* GetShaderCompiler();
	LoopStoper* GetLoopStopper();
	//LuaScriptManager* GetLuaScriptManager();
	SceneManager* GetSceneManager();
	const std::string& GetStartSceneFilePath() const { return startSceneFilePath_; }
	//LuaCallFiles* GetLuaCallFiles() { return &luaCallFiles_; }
	const DirectoryManager& GetDirectoryManager() { return directoryManager_; }

public:
	 float GetDeltaTime();

public:// デバッグ機能
#ifdef _DEBUG
	/// <summary>
	/// ゲームエンジンのメニューをImGuiで表示します
	/// </summary>
	void DrawEngineMenu();
#endif // _DEBUG

private:// デバッグ関数
	void SaveLunchConfig();

private:
	bool isDrawLunchConfigWindow_;
	std::string lunchConfigFileName_;
	nlohmann::json lunchConfig_;
	float systemSpundVolume_;

private:// コア機能
	WinApp winApp_;
	MSG* msg_;
	DirectXCommon dxCommon_;
	ImGuiManager imGuiManager_;
	LoopStoper loopStopper_;
	SceneManager sceneManager_;
	std::string startSceneFilePath_;

private:// リソース管理
	DirectoryManager directoryManager_;

private:// 時間管理
	FramePerSecond fpsCounter_;
	float deltaTime_;

private:// 描画機能
	ShaderCompiler shaderCompiler_;
	TextureManager textureManager_;
	GraphicsCommon graphicsCommon_;
	GraphRenderer graphRenderer_;
	RendaringPostprosecess postprocess_;

private:// 音声
	XAudioCore audioManager_;
	Audio3D audio3D_;
	AudioResourceManager audioResourceManager_;
	AudioSourceBinder audioSourceBinder_;
	AudioPlayer audioPlayer_;
	Chiptune chiptune_;

private:// 入力
	DirectInputManager inputManager_;
	XInputController xController_;
	
private:// ディスクリプタヒープ
	RtvDescriptorHeap rtvDescriptorHeap_;
	SrvDescriptorHeap srvDescriptorHeap_;

private:// デバッグ用一時変数
	//Sprite offscreen_;
	Transform offscreenTransform_;
	Transform uvTransform_;
	Camera camera_;

private:// スクリプト
	/*LuaScriptManager luaScriptManager_;
	LuaCallFiles luaCallFiles_;*/

#ifdef _DEBUG
	MemoryWatcher memWatcher_;
	bool isDrawMemoryDebugWindow_;

	MyTimer systemTimer_;
	bool isDrawFpsDebugWindow_;
	bool isDrawPerformanceDebugWindow_;
	float initializeRunningTime_;
	float updateRunningTime_;
	float drawRunningTime_;
	bool isDrawAudioSourceDebugWindow_;
	bool isDrawAudioDataDebugWindow_;
	bool isDrawPostprocessDebugWindow_;
	bool isDrawDebugLogWindow_;
	bool isDrawTextureDebugWindow_;
#endif // _DEBUG
};