#include "GameScene.h"

#include <cmath>

#ifndef XAUDIO2_PI
#define XAUDIO2_PI 3.1415926535f
#endif

GameScene::GameScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
	audio_ = engineCore->GetAudioManager();
	pan_ = 0.0f;
}

void GameScene::Initialize() {
	masterVolume_ = 0.5f;
	soundData_ = Audiomanager::SoundLoadWave("Resources/mono48kHz.wav");
	sourceVoice_ = Audiomanager::CreateSourceVoice(audio_->xAudio2_.Get(), soundData_);

	debugCamera_.Initialize(engineCore_->GetWinApp(), engineCore_->GetInputManager());
	obj_.Initialize(engineCore_);
	obj_.lookTargetPosition_ = &listener_.transform_.translate;
	listener_.Initialize(engineCore_);

	frameCount_ = 0.0f;
	cutoff_ = 2000.0f;
	cutoffRate_ = 0.5f;

	rotateSpeed_ = 0.05f;
}

void GameScene::Update() {
	frameCount_++;
	audio_->SetMasterVolume(masterVolume_);

	// パン
	pan_ = -obj_.transform_.translate.x;
	pan_ = std::clamp(pan_, -1.0f, 1.0f);
	matrix[0] = 0.5f - pan_ / 2.0f;
	matrix[1] = 0.5f + pan_ / 2.0f;
	sourceVoice_->SetOutputMatrix(audio_->GetMasterVoice(), 1, 2, matrix);

	// LPF
	XAUDIO2_FILTER_PARAMETERS filterParams;
	filterParams.Type = LowPassFilter;
	float cutoffFrequencyHz = cutoff_;
	float sampleRateHz = static_cast<float>(soundData_.wfex.nSamplesPerSec);

	float cutoffRate = (listener_.transform_.translate.Normalize() - obj_.transform_.translate.Normalize()).z * cutoffRate_;
	if (cutoffRate >= 0.0f) {
		cutoffRate = 0.0f;
	}

	cutoff_ = (sampleRateHz / 10.0f) + (sampleRateHz / 10.0f) * cutoffRate;
	if (cutoff_ <= 0.0f || cutoff_ > sampleRateHz) {
		assert(false);
	}

	cutoff_ = std::clamp(cutoff_, 0.0f, 4800.0f);

	filterParams.Frequency = 2.0f * sinf(XAUDIO2_PI * cutoffFrequencyHz / sampleRateHz);

	// Q値を設定 (ここではButterworthフィルターに近い値)
	filterParams.OneOverQ = 1.0f / 0.707f; // 約 1.414f

	HRESULT hr = sourceVoice_->SetFilterParameters(&filterParams);
	assert(SUCCEEDED(hr));

	// 音量
	if ((obj_.transform_.translate - listener_.transform_.translate).Length() != 0.0f) {
		float volume = 1.0f / (obj_.transform_.translate - listener_.transform_.translate).Length();
		sourceVoice_->SetVolume(volume);
	} else {
		sourceVoice_->SetVolume(1.0f);
	}

	// エミッターの移動
	obj_.transform_.translate.x = cosf(fabsf(sinf(frameCount_ * rotateSpeed_)) * 3.0f - 3.14f * 0.5f);
	obj_.transform_.translate.z = sinf(fabsf(sinf(frameCount_ * rotateSpeed_)) * 3.0f - 3.14f * 0.5f);

	if (Audiomanager::GetIsPlaying(sourceVoice_)) {
		obj_.transform_.scale.x = 1.0f - sinf(frameCount_ * 0.3f) * 0.1f;
		obj_.transform_.scale.y = 1.0f - cosf(frameCount_ * 0.3f) * 0.1f;
	} else {
		obj_.transform_.scale.x = 1.0f;
		obj_.transform_.scale.x = 1.0f;
	}

	obj_.Update();
	listener_.Update();
	debugCamera_.Update();
}

void GameScene::Draw() {
	ImGui::Begin("Audio");
	if (ImGui::Button("PlaySound")) {
		Audiomanager::SoundPlaySourceVoice(soundData_, sourceVoice_);
	}
	ImGuiKnobs::Knob("MasterVolume", &masterVolume_, 0.0f, 1.0f, 0.01f, "%.2fdB");
	ImGui::DragFloat("SpeakerRotateSpeed", &rotateSpeed_, 0.01f);
	ImGui::Text("%f", static_cast<float>(soundData_.wfex.nSamplesPerSec));

	ImGuiKnobs::Knob("Panning", &pan_, -1.0f, 1.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("Cutoff", &cutoff_, 0.0f, 48000.0f, 1.0f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("CutoffRate", &cutoffRate_, 0.0f, 1.0f, 0.01f, "%.2f");
	ImGui::End();

	ImGui::Begin("Emmiter");
	ImGui::DragFloat3("position", &obj_.transform_.translate.x, 0.1f);
	ImGui::DragFloat3("rotate", &obj_.transform_.rotate.x);
	ImGui::End();

	listener_.Draw(&debugCamera_.camera_);
	obj_.Draw(&debugCamera_.camera_);
}
