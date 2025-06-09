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
	//soundData_ = Audiomanager::SoundLoadMp3("Resources/shock.mp3");
	rightVoice_ = Audiomanager::CreateSourceVoice(audio_->xAudio2_.Get(), soundData_);
	leftVoice_ = Audiomanager::CreateSourceVoice(audio_->xAudio2_.Get(), soundData_);

	waterSoundData_ = Audiomanager::SoundLoadMp3("Resources/water.mp3");
	waterVoice_ = Audiomanager::CreateSourceVoice(audio_->xAudio2_.Get(), waterSoundData_);
	windSoundData_ = Audiomanager::SoundLoadMp3("Resources/wind.mp3");
	windVoice_ = Audiomanager::CreateSourceVoice(audio_->xAudio2_.Get(), windSoundData_);
	waterVoice_->SetVolume(0.5f);
	windVoice_->SetVolume(0.5f);

	mp3Data_ = Audiomanager::SoundLoadMp3("Resources/Enter.mp3");
	mp3Voice_ = Audiomanager::CreateSourceVoice(audio_->xAudio2_.Get(), mp3Data_);

	for (int i = 0; i < 256; i++) {
		echoVoice_.push_back(Audiomanager::CreateSourceVoice(audio_->xAudio2_.Get(), soundData_));
		echoPlayTime_.push_back(1.0f);
	}

	debugCamera_.Initialize(engineCore_->GetWinApp(), engineCore_->GetInputManager());
	obj_.Initialize(engineCore_);
	obj_.lookTargetPosition_ = &listener_.transform_.translate;
	listener_.Initialize(engineCore_);

	frameCount_ = 0.0f;
	cutoff_ = 2000.0f;
	cutoffRate_ = 0.5f;

	rotateSpeed_ = 0.05f;
	iidRate_ = 0.0f;
	iidVolume_ = 0.1f;
	internalVolumeDifferenceRate_ = 0.15f;

	isMove = true;

	echoTime_ = 0.0f;
	echoInterval_ = 1.0f;

	oldEchoIndex_ = 0;

	echoVolumeAttenuation_ = 0.2f;

	echoInitialVolumeAttenuation_ = 0.8f;

	echoRandMax_ = 1.0f;
	echoRandMin_ = 0.0f;

	cutoffOffset_ = 0.0f;

	playTimeRate_ = 1.0f;

	state_ = "Outside";

	playTimeDecayRate_ = 0.0f;
	echoInitialVolumeDecay_ = 1.0f;

	presetData_.emplace_back(cutoffOffset_);
	presetData_.emplace_back( cutoffRate_);
	presetData_.emplace_back( iidRate_);
	presetData_.emplace_back(iidVolume_);
	presetData_.emplace_back( internalVolumeDifferenceRate_);
	presetData_.emplace_back( echoVolumeAttenuation_);
	presetData_.emplace_back(echoInitialVolumeAttenuation_);
	presetData_.emplace_back(echoInitialVolumeDecay_);
	presetData_.emplace_back(echoRandMax_);
	presetData_.emplace_back(echoRandMin_);
	presetData_.emplace_back(playTimeDecayRate_);
	presetData_.emplace_back(echoInterval_);

	sprite_.Initialize(
		engineCore_->GetDirectXCommon(),
		engineCore_->GetTextureManager(),
		engineCore_->GetImGuiManager(),
		400.0f, 300.0f,
		engineCore_->GetGraphicsCommon()->GetTrianglePso(kBlendModeNone));
	sprite_.material_.materialData_->enableLighting = false;

	spriteTransform_.translate = { 1280.0f * 0.5f - 200.0f,720.0f * 0.5f - 150.0f,-50.0f };

	outsideGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/Outside.png");
	roomGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/Room.png");
	caveGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/Cave.png");
	waterGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/Water.png");
}

void GameScene::Update() {
	frameCount_++;
	audio_->SetMasterVolume(masterVolume_);

	// エミッターの移動
	if (isMove) {
		obj_.transform_.translate.x = cosf(fabsf(sinf(frameCount_ * rotateSpeed_)) * 3.0f - 3.14f * 0.5f);
		obj_.transform_.translate.z = sinf(fabsf(sinf(frameCount_ * rotateSpeed_)) * 3.0f - 3.14f * 0.5f);
		/*obj_.transform_.translate.x = cosf(frameCount_ * rotateSpeed_);
		obj_.transform_.translate.z = sinf(frameCount_ * rotateSpeed_);*/

		if (Audiomanager::GetIsPlaying(rightVoice_)) {
			obj_.transform_.scale.x = 1.0f - sinf(frameCount_ * 0.3f) * 0.1f;
			obj_.transform_.scale.y = 1.0f - cosf(frameCount_ * 0.3f) * 0.1f;
		} else {
			obj_.transform_.scale.x = 1.0f;
			obj_.transform_.scale.x = 1.0f;
		}
	}

	obj_.Update();
	listener_.Update();
	debugCamera_.Update();

	//* 音操作 *//
	// 両耳に音を振り分ける
	matrix[0] = 0.0f;
	matrix[1] = 1.0f;
	rightVoice_->SetOutputMatrix(audio_->GetMasterVoice(), 1, 2, matrix);
	matrix[0] = 1.0f;
	matrix[1] = 0.0f;
	leftVoice_->SetOutputMatrix(audio_->GetMasterVoice(), 1, 2, matrix);

	// 音量&パン
	pan_ = (listener_.transform_.translate - obj_.transform_.translate).Normalize().x;
	pan_ = std::clamp(pan_, -1.0f, 1.0f);

	if ((obj_.transform_.translate - listener_.transform_.translate).Length() != 0.0f) {
		float volume = 1.0f / (obj_.transform_.translate - listener_.transform_.translate).Length();
		float rightVolume = volume + (volume * +pan_) + (volume * -pan_) * internalVolumeDifferenceRate_;
		float leftVolume = volume + (volume * -pan_) + (volume * +pan_) * internalVolumeDifferenceRate_;
		rightVoice_->SetVolume(rightVolume);
		leftVoice_->SetVolume(leftVolume);

		nowRightVolume_ = rightVolume;
		nowLeftVolume_ = leftVolume;
	} else {
		rightVoice_->SetVolume(1.0f);
		leftVoice_->SetVolume(1.0f);
	}

	// LPF
	XAUDIO2_FILTER_PARAMETERS rightFilterParams;
	rightFilterParams.Type = LowPassFilter;
	XAUDIO2_FILTER_PARAMETERS leftFilterParams;
	leftFilterParams.Type = LowPassFilter;
	float sampleRateHz = static_cast<float>(soundData_.wfex.nSamplesPerSec);

	float cutoffRate = (listener_.transform_.translate.Normalize() - obj_.transform_.translate.Normalize()).z * cutoffRate_;
	if (cutoffRate >= 0.0f) {
		cutoffRate = 0.0f;
	}

	cutoff_ = (sampleRateHz / 10.0f) + (sampleRateHz / 10.0f) * cutoffRate;
	cutoff_ -= cutoffOffset_;
	cutoff_ = std::clamp(cutoff_, 0.0f, 4800.0f);

	if (cutoff_ <= 0.0f || cutoff_ > sampleRateHz) {
		assert(false);
	}

	// iid
	iidRate_ = (cutoff_ * pan_) * iidVolume_;
	float rightCutoffFrequencyHz = cutoff_ + iidRate_;
	float leftCutoffFrequencyHz = cutoff_ + iidRate_;

	rightFilterParams.Frequency = 2.0f * sinf(XAUDIO2_PI * rightCutoffFrequencyHz / sampleRateHz);
	leftFilterParams.Frequency = 2.0f * sinf(XAUDIO2_PI * leftCutoffFrequencyHz / sampleRateHz);

	// Q値を設定 (ここではButterworthフィルターに近い値)
	rightFilterParams.OneOverQ = 1.0f / 0.707f; // 約 1.414f
	leftFilterParams.OneOverQ = 1.0f / 0.707f; // 約 1.414f

	HRESULT hr = rightVoice_->SetFilterParameters(&rightFilterParams);
	assert(SUCCEEDED(hr));
	 hr = leftVoice_->SetFilterParameters(&leftFilterParams);
	assert(SUCCEEDED(hr));

	// エコー
	if (isEcho_) {
		if (echoTime_ > 0.0f) {
			echoTime_ -= engineCore_->GetDeltaTime();
		} else {
			echoTime_ = echoInterval_ + echoIntervalRand_.GetUniformDistributionRand(echoRandMin_, echoRandMax_);

			bool isPlay = false;

			for (IXAudio2SourceVoice* sourceVoice : echoVoice_) {

				if (!Audiomanager::GetIsPlaying(sourceVoice) && Audiomanager::GetIsPlaying(rightVoice_)) {
					matrix[0] = 0.0f;
					matrix[1] = 1.0f;
					sourceVoice->SetOutputMatrix(audio_->GetMasterVoice(), 1, 2, matrix);

					sourceVoice->SetVolume(nowRightVolume_ * echoInitialVolumeAttenuation_);
					hr = sourceVoice->SetFilterParameters(&rightFilterParams);
					assert(SUCCEEDED(hr));
					Audiomanager::SoundPlaySourceVoice(soundData_, sourceVoice);
					isPlay = true;
					break;
				}
			}

			for (IXAudio2SourceVoice* sourceVoice : echoVoice_) {
				if (!Audiomanager::GetIsPlaying(sourceVoice) && Audiomanager::GetIsPlaying(leftVoice_)) {
					matrix[0] = 1.0f;
					matrix[1] = 0.0f;
					sourceVoice->SetOutputMatrix(audio_->GetMasterVoice(), 1, 2, matrix);

					sourceVoice->SetVolume(nowLeftVolume_* echoInitialVolumeAttenuation_);
					hr = sourceVoice->SetFilterParameters(&leftFilterParams);
					assert(SUCCEEDED(hr));
					Audiomanager::SoundPlaySourceVoice(soundData_, sourceVoice);
					isPlay = true;
					break;
				}
			}

			if (isPlay) {
				echoInitialVolumeAttenuation_ *= echoInitialVolumeDecay_;
			}
		}
	}

	// 減衰
	for (IXAudio2SourceVoice* sourceVoice : echoVoice_) {
		if (Audiomanager::GetIsPlaying(sourceVoice)) {
			float volume = 0.0f;
			sourceVoice->GetVolume(&volume);
			sourceVoice->SetVolume(volume * (1.0f - echoVolumeAttenuation_ * engineCore_->GetDeltaTime()));
		}
	}

	// 再生速度減衰
	int index = 0;
	for (IXAudio2SourceVoice* sourceVoice : echoVoice_) {
		if (Audiomanager::GetIsPlaying(sourceVoice)) {
			echoPlayTime_[index] *= 1.0f - playTimeDecayRate_ * engineCore_->GetDeltaTime();
			sourceVoice->SetFrequencyRatio(echoPlayTime_[index]);

		}
		index++;
	}
}

void GameScene::Draw() {
	if (state_ == "Outside") {
		sprite_.DrawSprite(spriteTransform_, outsideGH_, &debugCamera_.camera_);
	} else if (state_ == "Room") {
		sprite_.DrawSprite(spriteTransform_, roomGH_, &debugCamera_.camera_);
	} else if (state_ == "Cave") {
		sprite_.DrawSprite(spriteTransform_, caveGH_, &debugCamera_.camera_);
	} else if (state_ == "UnderTheWater") {
		sprite_.DrawSprite(spriteTransform_,waterGH_,&debugCamera_.camera_);
	}

	ImGui::Begin("Preset");
	ImGui::Text("State: %s", state_.c_str());
	if (ImGui::Button("OutSide")) {
		state_ = "Outside";
		Audiomanager::SoundPlaySourceVoice(mp3Data_, mp3Voice_);
		windVoice_->SetFrequencyRatio(1.0f);
		Audiomanager::SoundPlaySourceVoice(windSoundData_, windVoice_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Room")) {
		state_ = "Room";
		Audiomanager::SoundPlaySourceVoice(mp3Data_, mp3Voice_);

		windVoice_->Stop();
		waterVoice_->Stop();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cave")) {
		state_ = "Cave";
		Audiomanager::SoundPlaySourceVoice(mp3Data_, mp3Voice_);

		windVoice_->Stop();
		waterVoice_->Stop();

		windVoice_->SetFrequencyRatio(0.1f);
		Audiomanager::SoundPlaySourceVoice(windSoundData_, windVoice_);
	}
	ImGui::SameLine();
	if (ImGui::Button("UnderTheWater")) {
		state_ = "UnderTheWater";
		Audiomanager::SoundPlaySourceVoice(mp3Data_, mp3Voice_);

		windVoice_->Stop();
		Audiomanager::SoundPlaySourceVoice(waterSoundData_, waterVoice_);
	}
	if (ImGui::Button("Save")) {
		presetData_[0] = cutoffOffset_;
		presetData_[1] = cutoffRate_;
		presetData_[2] = iidRate_;
		presetData_[3] = iidVolume_;
		presetData_[4] = internalVolumeDifferenceRate_;
		presetData_[5] = echoVolumeAttenuation_;
		presetData_[6] = echoInitialVolumeAttenuation_;
		presetData_[7] = echoInitialVolumeDecay_;
		presetData_[8] = echoRandMax_;
		presetData_[9] = echoRandMin_;
		presetData_[10] = playTimeDecayRate_;
		presetData_[11] = echoInterval_;
		SJN::SaveJsonData(state_ + "Preset", presetData_);
		Audiomanager::SoundPlaySourceVoice(mp3Data_, mp3Voice_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		presetData_ = SJN::LoadJsonData("Resources/GlobalVariables/" + state_ + "Preset.json");
		cutoffOffset_ = presetData_[0];
		cutoffRate_ = presetData_[1];
		iidRate_ = presetData_[2];
		iidVolume_ = presetData_[3];
		internalVolumeDifferenceRate_ = presetData_[4];
		echoVolumeAttenuation_ = presetData_[5];
		echoInitialVolumeAttenuation_ = presetData_[6];
		echoInitialVolumeDecay_ = presetData_[7];
		echoRandMax_ = presetData_[8];
		echoRandMin_ = presetData_[9];
		playTimeDecayRate_ = presetData_[10];
		echoInterval_ = presetData_[11];
		Audiomanager::SoundPlaySourceVoice(mp3Data_, mp3Voice_);
	}
	ImGui::End();

	ImGui::Begin("Audio");

	if (ImGui::Button("PlaySound")) {
		Audiomanager::SoundPlaySourceVoice(soundData_, rightVoice_);
		Audiomanager::SoundPlaySourceVoice(soundData_, leftVoice_);

		for (float& playTime : echoPlayTime_) {
			playTime = 1.0f;
		}
	}
	if (ImGui::Button("isMoved")) {
		isMove = !isMove;
	}
	if (!isMove) {
		ImGui::DragFloat3("emitterPos", &obj_.transform_.translate.x,0.1f);
	}
	ImGuiKnobs::Knob("MasterVolume", &masterVolume_, 0.0f, 1.0f, 0.01f, "%.2fdB");
	ImGui::DragFloat("SpeakerRotateSpeed", &rotateSpeed_, 0.01f);
	ImGui::Text("%f", static_cast<float>(soundData_.wfex.nSamplesPerSec));

	ImGuiKnobs::Knob("Panning", &pan_, -1.0f, 1.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("CutoffOffset", &cutoffOffset_, 0.0f, 2000.0f, 1.0f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("Cutoff", &cutoff_, 0.0f, 48000.0f, 1.0f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("CutoffRate", &cutoffRate_, 0.0f, 1.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("IID", &iidVolume_, 0.0f, 1.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("InternalVolumeDifferenceRate_", &internalVolumeDifferenceRate_, 0.0f, 1.0f, 0.01f, "%.2f");

	if (ImGui::Button("isEcho")) {
		isEcho_ = !isEcho_;
	}
	ImGuiKnobs::Knob("echo", &echoTime_, 0.1f, 5.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("echointerval", &echoInterval_, 0.0f, 5.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("echoVolumeA", &echoVolumeAttenuation_, 0.0f, 1.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("echoInitVolumeA", &echoInitialVolumeAttenuation_, 0.0f, 1.0f, 0.01f, "%.2f");

	ImGui::SameLine();
	ImGuiKnobs::Knob("echoMax", &echoRandMax_, 0.0f, 1.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("echomin", &echoRandMin_, 0.0f, 1.0f, 0.01f, "%.2f");

	ImGuiKnobs::Knob("echoPlayTimeDecay", &playTimeDecayRate_, 0.0f, 1.0f, 0.01f, "%.2f");
	ImGui::SameLine();
	ImGuiKnobs::Knob("echoInitalVolumeDecay", &echoInitialVolumeDecay_, 0.0f, 1.0f, 0.01f, "%.2f");
	ImGui::End();

	ImGui::Begin("Emmiter");
	ImGui::DragFloat3("position", &obj_.transform_.translate.x, 0.1f);
	ImGui::DragFloat3("rotate", &obj_.transform_.rotate.x);
	ImGui::End();

	listener_.Draw(&debugCamera_.camera_);
	obj_.Draw(&debugCamera_.camera_);
}

void GameScene::EchoVoice() {

	Audiomanager::SoundPlaySourceVoice(soundData_, rightVoice_);
	Audiomanager::SoundPlaySourceVoice(soundData_, leftVoice_);
}
