#include "GameScene.h"

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
	listener_.Initialize(engineCore_);

	frameCount_ = 0.0f;
}

void GameScene::Update() {
	frameCount_++;
	audio_->SetMasterVolume(masterVolume_);

	matrix[0] = 0.5f - pan_ / 2.0f;
	matrix[1] = 0.5f + pan_ / 2.0f;

	sourceVoice_->SetOutputMatrix(audio_->GetMasterVoice(), 1, 2, matrix);

	obj_.transform_.translate.x = pan_;

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
	ImGuiKnobs::Knob("MasterVolume", &masterVolume_, 0.0f, 1.0f, 0.01f, "%.2fdB");
	ImGuiKnobs::Knob("Panning", &pan_, -1.0f, 1.0f, 0.01f, "%.2f");
	if (ImGui::Button("PlaySound")) {
		Audiomanager::SoundPlaySourceVoice(soundData_, sourceVoice_);
	}
	ImGui::End();

	ImGui::Begin("Listener");
	ImGui::End();

	listener_.Draw(&debugCamera_.camera_);
	obj_.Draw(&debugCamera_.camera_);
}
