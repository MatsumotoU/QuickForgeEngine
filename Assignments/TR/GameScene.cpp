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
	obj_.lookTargetPosition_ = &listener_.transform_.translate;
	listener_.Initialize(engineCore_);

	frameCount_ = 0.0f;
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

	// 音量
	if ((obj_.transform_.translate - listener_.transform_.translate).Length() != 0.0f) {
		float volume = 1.0f / (obj_.transform_.translate - listener_.transform_.translate).Length();
		sourceVoice_->SetVolume(volume);
	} else {
		sourceVoice_->SetVolume(1.0f);
	}

	// エミッターの移動
	obj_.transform_.translate.x = cosf(frameCount_*0.1f);
	obj_.transform_.translate.z = sinf(frameCount_*0.1f);

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

	ImGui::Begin("Emmiter");
	ImGui::DragFloat3("position", &obj_.transform_.translate.x,0.1f);
	ImGui::DragFloat3("rotate", &obj_.transform_.rotate.x);
	ImGui::End();

	listener_.Draw(&debugCamera_.camera_);
	obj_.Draw(&debugCamera_.camera_);
}
