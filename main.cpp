#include "Engine/Base/Windows/WinApp.h"
#include "Engine/Base/DirectX/DirectXCommon.h"
#include "Engine/Base/DirectX/shaderCompiler.h"
#include "Engine/Base/DirectX/ImGuiManager.h"
#include "Engine/Base/DirectX/TextureManager.h"
// PSO
#include "Engine/Base/DirectX/PipelineStateObject.h"

#include "Engine/Base/MyString.h"

// Math
#include "Engine/Math/Matrix/Matrix4x4.h"
#include "Engine/Math/Vector/Vector4.h"
#include "Engine/Math/Transform.h"
#include "Engine/Math/VerTexData.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

// CrashHandler
#include <dbghelp.h>
#include <strsafe.h>
#pragma comment(lib,"Dbghelp.lib")

int windowWidth = 1280;
int windowHeight = 720;

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのID)とクラッシュ(例外)の発生したthredIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
	minidumpInfomation.ThreadId = threadId;
	minidumpInfomation.ExceptionPointers = exception;
	minidumpInfomation.ClientPointers = TRUE;
	// Dumpを出力
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
	// 他に関連付けられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する
	return EXCEPTION_EXECUTE_HANDLER;
}

// windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// * ゲーム以前の設定 * //
	// ダンプ渡し
	SetUnhandledExceptionFilter(ExportDump);

	// ウィンドウ生成
	WinApp* winApp = WinApp::GetInstance();
	winApp->CreateGameWindow(windowWidth, windowHeight);
	MSG msg{};

	// DirectXの初期化
	DirectXCommon* dxCommon = DirectXCommon::GetInstatnce();
	dxCommon->Initialize();

	// ImGuiの初期化
	ImGuiManager* imGuiManager = ImGuiManager::GetInstatnce();
	imGuiManager->Initialize(winApp,dxCommon);

	// TextManagerの初期化
	TextureManager* textureManager = TextureManager::GetInstatnce();
	textureManager->Initialize(dxCommon->GetDevice());

	// * PSOを作成 * //
	PipelineStateObject pso;
	pso.Initialize();

	// * ゲーム内の設定 * //
	// 変数定義
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };

	// テクスチャを読み込む
	DirectX::ScratchImage mipImages = textureManager->LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	ID3D12Resource* textureResource = textureManager->CreateTextureResource(metadata);
	textureManager->CreateShaderResourceView(metadata, imGuiManager->GetSrvDescriptorHeap(), textureResource);

	// マテリアル用のリソースを作る
	ID3D12Resource* materialResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Vector4));
	// マテリアルにデータを書き込む
	Vector4* materialData = nullptr;
	// 書き込むためのアドレス取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	ID3D12Resource* wvpResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));
	// データを書き込む
	Matrix4x4* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	*wvpData = Matrix4x4::MakeIndentity4x4();

	// * VertexResourceを生成する * //
	ID3D12Resource* vertexResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(VetrtexData) * 6);

	// * VertexBufferViewを作成する * //
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VetrtexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(VetrtexData);

	// * リソースにデータを書き込む * //
	VetrtexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f};
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	vertexData[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	vertexData[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	vertexData[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };

	// * ビューポートとシザー * //
	D3D12_VIEWPORT viewport{};
	viewport.Width = 1280;
	viewport.Height = 720;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = 1280;
	scissorRect.top = 0;
	scissorRect.bottom = 720;

	// ウィンドウのXボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// windowにメッセージが基底たら最優先で処理される
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {

			transform.rotate.y += 0.01f;

			Matrix4x4 cameraMatrix = Matrix4x4::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 viewMatrix = Matrix4x4::Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(0.45f, static_cast<float>(winApp->kWindowWidth) / static_cast<float>(winApp->kWindowHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix, projectionMatrix));

			*wvpData = worldViewProjectionMatrix;

			dxCommon->PreDraw();
			imGuiManager->BeginFrame();

			// テクスチャ用のリソース
			ID3D12Resource* intermediateResource = textureManager->UploadTextureData(textureResource, mipImages, dxCommon->GetCommandList());

			ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

			// Depthの設定
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = pso.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
			commandList->OMSetRenderTargets(1, dxCommon->GetRtvHandles(), false, &dsvHandl);
			commandList->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissorRect);
			commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			commandList->SetPipelineState(pso.GetPipelineState());
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureSrvHandleGPU());
			commandList->DrawInstanced(6, 1, 0, 0);

			ImGui::ShowDemoWindow();

			imGuiManager->EndFrame();
			dxCommon->PostDraw();

			intermediateResource->Release();
		}
	}

	// * 解放処理 * //
	materialResource->Release();
	wvpResource->Release();

	vertexResource->Release();

	imGuiManager->EndImGui();
	dxCommon->ReleaseDirectXObject();
	CloseWindow(winApp->GetHWND());

	textureManager->Finalize();

	// * 終了時のエラー処理 * //
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	return 0;
}