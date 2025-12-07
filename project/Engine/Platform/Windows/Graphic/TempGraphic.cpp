#include "TempGraphic.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Renderer/GraphRenderer.h"
#include "Core/Math/AudioMath.h"

#include "Assets/AssetManager.h"
#include "Assets/AudioSource/AudioSourceManager.h"
#include "Assets/AudioSource/Data/Spectrum.h"

void TempGraphic::Initialize() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	echoSphereBuffer_.CreateResource(DirectXCommon::GetInstance()->GetDevice());
	echoSphereBuffer_.GetData()->count = kMaxEchoSpheres;
	echoSphereBuffer_.GetData()->isUse = true;
	pow = 0.0f;

	echoSphereStructuredBuffer_.CreateResource(
		dxCommon,
		dxCommon->GetDescriptorHeapManager()->GetSrvDescriptorHeap(),
		kMaxEchoSpheres);
	

	for (int i = 0; i < kMaxEchoSpheres; i++) {
		echoSphereStructuredBuffer_.GetData()[i].isActive = false;
		echoSphereStructuredBuffer_.GetData()[i].sphereRadius = 0.0f;
		echoSphereStructuredBuffer_.GetData()[i].sphereThickness = 0.0f;
		echoSphereStructuredBuffer_.GetData()[i].sphereCenter = Vector3(0.0f, 0.0f, 0.0f);
		echoSphereStructuredBuffer_.GetData()[i].alpha = 0.0f;
	}
}

void TempGraphic::Update() {
	for (int i = 0; i < kMaxEchoSpheres; i++) {
		if (!echoSphereStructuredBuffer_.GetData()[i].isActive) {
			continue;
		}

		// expandSpeedを使う
		echoSphereStructuredBuffer_.GetData()[i].sphereRadius +=echoSphereStructuredBuffer_.GetData()[i].expandSpeed;

		if (echoSphereStructuredBuffer_.GetData()[i].isActive && echoSphereStructuredBuffer_.GetData()[i].sphereThickness > 0.0f) {
			echoSphereStructuredBuffer_.GetData()[i].sphereThickness -= 0.01f;
			if (echoSphereStructuredBuffer_.GetData()[i].sphereThickness < 0.0f) {
				echoSphereStructuredBuffer_.GetData()[i].sphereThickness = 0.0f;
				echoSphereStructuredBuffer_.GetData()[i].isActive = false;
			}
		}
	}
}

void TempGraphic::Draw() {

	/*ImGui::Begin("TempGraphic");
	ImGui::DragFloat3("SphereCenter",
		&echoSphereBuffer_.GetData()->sphereCenter.x, 0.1f);
	ImGui::DragFloat("SphereRadius",
		&echoSphereBuffer_.GetData()->sphereRadius, 0.1f, 0.0f);
	ImGui::DragFloat("SphereThickness",
		&echoSphereBuffer_.GetData()->sphereThickness, 0.01f, 0.0f);
	ImGui::End();

	if (echoSphereBuffer_.GetData()->sphereRadius > 0.0f) {
		GraphRenderer::GetInstance()->DrawSphere(
			echoSphereBuffer_.GetData()->sphereCenter,
			echoSphereBuffer_.GetData()->sphereRadius,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f), 16);
	}*/
	
}

void TempGraphic::Finalize() {
	
}

void TempGraphic::Echo(Vector3 pos, float thickness, float expandSpeed) {
	for (int i = 0; i < kMaxEchoSpheres; i++) {
		if (!echoSphereStructuredBuffer_.GetData()[i].isActive) {
			echoSphereStructuredBuffer_.GetData()[i].isActive = true;
			echoSphereStructuredBuffer_.GetData()[i].sphereCenter = pos;
			echoSphereStructuredBuffer_.GetData()[i].sphereRadius = 0.0f;
			echoSphereStructuredBuffer_.GetData()[i].sphereThickness = thickness;
			echoSphereStructuredBuffer_.GetData()[i].expandSpeed = expandSpeed; // 新規追加
			echoSphereStructuredBuffer_.GetData()[i].alpha = 0.1f;
			break;
		}
	}
}

void TempGraphic::EchoFromAudioData(uint32_t audioHandle, Vector3 pos, float power)
{
	AssetManager* assetManager = AssetManager::GetInstance();
	AudioSourceManager* audioSourceManager = assetManager->GetAudioSourceManager();
	Spectrum s = MyAudioMath::CreateSpectrumFromAudioData(audioSourceManager->GetSoundData(audioHandle));
	// magnitudeが0.8以上の要素だけを抽出
	std::vector<std::pair<float, float>> freqMagPairs;
	for (size_t i = 0; i < s.magnitudes.size(); ++i) {
		if (s.magnitudes[i] >= 100.0f) {
			freqMagPairs.emplace_back(s.frequencies[i], s.magnitudes[i]);
		}
	}

	// magnitudeの大きい順にソート
	std::sort(freqMagPairs.begin(), freqMagPairs.end(),
		[](const std::pair<float, float>& a, const std::pair<float, float>& b) {
			return a.second > b.second;
		});

	// 10個を超える場合は等間隔で10個選ぶ
	size_t sampleCount = 3;
	if (freqMagPairs.size() > sampleCount) {
		std::vector<std::pair<float, float>> selectedPairs;
		size_t step = freqMagPairs.size() / sampleCount;
		for (size_t i = 0; i < sampleCount; ++i) {
			size_t idx = i * step;
			if (idx >= freqMagPairs.size()) idx = freqMagPairs.size() - 1;
			selectedPairs.push_back(freqMagPairs[idx]);
		}
		freqMagPairs = selectedPairs;
	}

	for (const auto& pair : freqMagPairs) {
		float freq = pair.first;
		float normFreq = freq / 20000.0f; // 0.0～1.0に正規化

		// 高周波数ほど範囲が広く、幅が狭い
		float expandSpeed = 0.05f + (normFreq * normFreq); // 0.05～0.2
		float thickness = 0.1f + (1.0f - normFreq) * 1.1f * power; // 低周波数ほど幅が広い

		Echo(pos, thickness, expandSpeed);
	}
}
