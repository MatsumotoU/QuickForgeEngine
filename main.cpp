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

// DebugLog
#include "Engine/Base/MyDebugLog.h"

int windowWidth = 1280;
int windowHeight = 720;

// CPU側の設定？シェーダーはGPU？
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
};

struct DirectionalLight
{
	Vector4 color;// ライトの色
	Vector3 direction;// ライトの向き
	float intensity;// 輝度
};

//srv,rtv,dsvはimGuiManager,dxCommon,textureManagerが持っているので要分離(こいつらは増えない)

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
	// デバッグログ用
	MyDebugLog* myDebugLog = MyDebugLog::GetInstatnce();
	myDebugLog->Initialize();

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
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	int32_t ballTextureIndex = 0;

	// テクスチャを読み込む
	DirectX::ScratchImage mipImages = textureManager->LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	ID3D12Resource* textureResource = textureManager->CreateTextureResource(metadata);
	textureManager->CreateShaderResourceView(metadata, imGuiManager->GetSrvDescriptorHeap(), textureResource,1);
	// 2米
	DirectX::ScratchImage mipImages2 = textureManager->LoadTexture("Resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	ID3D12Resource* textureResource2 = textureManager->CreateTextureResource(metadata2);
	textureManager->CreateShaderResourceView(metadata2, imGuiManager->GetSrvDescriptorHeap(), textureResource2,2);

	// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	ID3D12Resource* wvpResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	wvpData->WVP = Matrix4x4::MakeIndentity4x4();
	wvpData->World = Matrix4x4::MakeIndentity4x4();

	// マテリアル用のリソースを作る
	ID3D12Resource* materialResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// 書き込むためのアドレス取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;

	// 光源用のリソースを作る。DirectionnalLight一つ分のサイズを用意する
	ID3D12Resource* directionnalLightResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));
	// データを書き込む
	DirectionalLight* directionnalLightData = nullptr;
	// 書き込むためのアドレスを取得
	directionnalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionnalLightData));
	// 情報を書き込んでおく
	directionnalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionnalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionnalLightData->intensity = 1.0f;

	// * VertexResourceを生成する * //
	const int32_t kSubdivision = 16;
	const int32_t sphereVertexMax = kSubdivision * kSubdivision * 6;
	ID3D12Resource* vertexResource = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * sphereVertexMax);

	// * VertexBufferViewを作成する * //
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * sphereVertexMax;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// * リソースにデータを書き込む * //
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	/*vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
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
	vertexData[5].texcoord = { 1.0f,1.0f };*/

	// Spriteを作る
	ID3D12Resource* vertexResourceSprite = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * 6);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };

	vertexDataSprite[3].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[3].texcoord = { 0.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[4].texcoord = { 1.0f,0.0f };
	vertexDataSprite[4].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[5].texcoord = { 1.0f,1.0f };
	vertexDataSprite[5].normal = { 0.0f,0.0f,-1.0f };

	// マテリアル用のリソースを作る
	ID3D12Resource* materialResourceSprite = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	// 書き込むためのアドレス取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// 今回は赤を書き込んでみる
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite->enableLighting = false;

	// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	ID3D12Resource* transformationMatrixResourceSprite = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// 単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = Matrix4x4::MakeIndentity4x4();
	transformationMatrixDataSprite->World = Matrix4x4::MakeIndentity4x4();

	// 球
	const float pi = DirectX::XM_PI;
	
	const float kLonEvery = pi * 2.0f / static_cast<float>(kSubdivision);
	const float kLatEvery = pi / static_cast<float>(kSubdivision);
	for (int32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * static_cast<float>(latIndex);
		for (int32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = static_cast<float>(lonIndex) * kLonEvery;

			Vector4 a = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon),1.0f };
			Vector4 b = { cos(lat + kLatEvery) * cos(lon),sin(lat+ kLatEvery),cos(lat+ kLatEvery) * sin(lon),1.0f };
			Vector4 c = { cos(lat) * cos(lon+ kLonEvery),sin(lat),cos(lat) * sin(lon+ kLonEvery),1.0f };
			Vector4 d = { cos(lat + kLatEvery) * cos(lon + kLonEvery),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon + kLonEvery),1.0f };

			Vector3 aNormal = { a.x,a.y,a.z };
			Vector3 bNormal = {	b.x,b.y,b.z };
			Vector3 cNormal = { c.x,c.y,c.z };
			Vector3 dNormal = { d.x,d.y,d.z };

			vertexData[start].position = a;
			vertexData[start].texcoord = { 
				static_cast<float>(lonIndex) / static_cast<float>(kSubdivision),
				1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision) };
			vertexData[start].normal = aNormal;
			vertexData[start + 1].position = b;
			vertexData[start + 1].texcoord = { 
				static_cast<float>(lonIndex) / static_cast<float>(kSubdivision),
				1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision) };
			vertexData[start+1].normal = bNormal;
			vertexData[start + 2].position = c;
			vertexData[start + 2].texcoord = { 
				static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision),
				1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision) };
			vertexData[start+2].normal = cNormal;

			vertexData[start + 3].position = b;
			vertexData[start + 3].texcoord = {
				static_cast<float>(lonIndex) / static_cast<float>(kSubdivision),
				1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision) };
			vertexData[start + 3].normal = bNormal;
			vertexData[start + 4].position = d;
			vertexData[start + 4].texcoord = {
				static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision),
				1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision) };
			vertexData[start + 4].normal = dNormal;
			vertexData[start + 5].position = c;
			vertexData[start + 5].texcoord = {
				static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision),
				1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision) };
			vertexData[start + 5].normal = cNormal;
		}
	}

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

	bool Lighting = true;

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
			Matrix4x4 worldMatrixSprite = Matrix4x4::MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrix = Matrix4x4::Inverse(cameraMatrix);
			Matrix4x4 viewMatrixSprite = Matrix4x4::MakeIndentity4x4();
			Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(0.45f, static_cast<float>(winApp->kWindowWidth) / static_cast<float>(winApp->kWindowHeight), 0.1f, 100.0f);
			Matrix4x4 projectionMatrixSprite = Matrix4x4::MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(winApp->kWindowWidth), static_cast<float>(winApp->kWindowHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix, projectionMatrix));
			Matrix4x4 worldViewProjectionMatrixSprite = Matrix4x4::Multiply(worldMatrixSprite, Matrix4x4::Multiply(viewMatrixSprite, projectionMatrixSprite));

			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
			transformationMatrixDataSprite->World = worldMatrixSprite;
			wvpData->WVP = worldViewProjectionMatrix;
			wvpData->World = worldMatrix;

			dxCommon->PreDraw();
			imGuiManager->BeginFrame();

			// テクスチャをvramにアップロード
			ID3D12Resource* intermediateResource = textureManager->UploadTextureData(textureResource, mipImages, dxCommon->GetCommandList());
			ID3D12Resource* intermediateResource2 = textureManager->UploadTextureData(textureResource2, mipImages2, dxCommon->GetCommandList());

			ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

			// Depthの設定
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = pso.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
			commandList->OMSetRenderTargets(1, dxCommon->GetRtvHandles(), false, &dsvHandl);
			commandList->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			// 球体
			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissorRect);
			commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			commandList->SetPipelineState(pso.GetPipelineState());
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(3, directionnalLightResource->GetGPUVirtualAddress());
			
			commandList->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureSrvHandleGPU(ballTextureIndex));
			commandList->DrawInstanced(sphereVertexMax, 1, 0, 0);

			// sprite
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureSrvHandleGPU(0));

			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			commandList->DrawInstanced(6, 1, 0, 0);

			ImGui::InputInt("textureIndex", &ballTextureIndex, 1);
			if (ballTextureIndex < 0) {
				ballTextureIndex = 0;
			}
			if (ballTextureIndex > 1) {
				ballTextureIndex = 1;
			}
			ImGui::DragFloat3("cametaTransition", &cameraTransform.translate.x,0.1f);
			ImGui::DragFloat3("cametaRotate", &cameraTransform.rotate.x, 0.1f);
			ImGui::DragFloat3("directionnalLight.direction", &directionnalLightData->direction.x, 0.1f);
			ImGui::Checkbox("isLighting", &Lighting);
			materialData->enableLighting = Lighting;
			directionnalLightData->direction = directionnalLightData->direction.Normalize();

			imGuiManager->EndFrame();
			dxCommon->PostDraw();

			intermediateResource->Release();
			intermediateResource2->Release();
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
	myDebugLog->Finalize();

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