#include "TitleScene.h"
#include "GameScene.h"
#include "Utility/MyEasing.h"
#include "Utility/RotateToDir.h"

TitleScene::TitleScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
}

TitleScene::~TitleScene() {
	engineCore_->GetAudioPlayer()->StopAudio("TitleBgm.mp3");
}

void TitleScene::Initialize() {
	cameraShakePower_ = 0.0f;
	isRequestedExit_ = false;
	mainTitlemodel_.Initialize(engineCore_);
	mainTitlemodel_.LoadModel("Resources", "MainTitle.obj", COORDINATESYSTEM_HAND_RIGHT);
	mainTitlemodel_.material_.materialData_->color = { 0.85f,0.0f,0.05f ,1.0f };
	subTitleModel_.Initialize(engineCore_);
	subTitleModel_.LoadModel("Resources", "SubTitle.obj", COORDINATESYSTEM_HAND_RIGHT);
	camera_.Initialize(engineCore_->GetWinApp());

	mainTitleTransform_.translate = { 0.8f,0.2f,0.0f };
	mainTitleTransform_.rotate = { -0.3f,3.4f,-0.2f };
	subTitleTransform_.translate = { -1.5f,-1.4f,3.9f };
	subTitleTransform_.rotate = { -0.25f,-3.4f,0.1f };

	mainTitleBeatRate_ = 53;
	subTitleBeatRate_ = 27;

	ice_.Initialize(engineCore_);
	ice_.transform_.translate = { -1.0f,0.0f,0.0f };
	ice_.syropeColor_ = { 1.0f,0.0f,0.0f,1.0f };

	azarasi_.Initialize(engineCore_);
	azarasi_.transform_.translate = { 0.8f,-0.4f,-3.2f };
	azarasi_.transform_.rotate = { 0.0f,-4.0f,-0.3f };
	azarasiMoveDir_ = RotateToDir::GetRotateToDir(azarasi_.transform_.rotate);
	isAzarasiMove_ = false;

	titleBgmHandle_ = engineCore_->LoadSoundData("Resources/", "TitleBgm.mp3");
	engineCore_->GetAudioPlayer()->PlayAudio(titleBgmHandle_, "TitleBgm.mp3", true);
	cameraShakePower_ = 0.3f;
	transitionFrame_ = 60;
}

void TitleScene::Update() {
	DirectInputManager* input = engineCore_->GetInputManager();
	frameCount_++;

	Eas::SimpleEaseIn(&mainTitleTransform_.scale.x, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&mainTitleTransform_.scale.y, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&subTitleTransform_.scale.x, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&subTitleTransform_.scale.y, 1.0f, 0.1f);

	mainTitleTransform_.translate.y = 0.2f + (std::sin(frameCount_ * 0.11f) * 0.05f);
	subTitleTransform_.translate.y = -1.4f + (std::sin(frameCount_ * 0.11f) * 0.05f);

	if (static_cast<int>(frameCount_) % mainTitleBeatRate_ == 0) {
		mainTitleTransform_.scale.x = 0.5f;
		mainTitleTransform_.scale.y = 1.5f;

	}
	if (static_cast<int>(frameCount_) % subTitleBeatRate_ == 0) {
		subTitleTransform_.scale.x = 0.8f;
		subTitleTransform_.scale.y = 1.3f;

		ice_.transform_.scale.y = 1.5f;
		azarasi_.transform_.scale.x = 1.2f;
		azarasi_.transform_.scale.y = 1.2f;
		azarasi_.transform_.scale.z = 1.1f;
	}

	if (input->keyboard_.GetPress(DIK_SPACE)) {
		//isRequestedExit_ = true;
		azarasi_.reqestMouthOpen_ = true;

	} else {
		if (azarasi_.reqestMouthOpen_) {
			azarasi_.reqestMouthOpen_ = false;
			if (azarasi_.transform_.scale.x > 0.8f) {
				isAzarasiMove_ = true;
			}
		}
	}

	if (isAzarasiMove_) {
		azarasi_.transform_.translate.x -= azarasiMoveDir_.x * 0.2f;
		azarasi_.transform_.translate.y -= azarasiMoveDir_.y * 0.2f;
		azarasi_.transform_.translate.z -= azarasiMoveDir_.z * 0.2f;
		if (azarasi_.transform_.translate.z > 5.0f) {
			
			if (transitionFrame_ > 0) {
				transitionFrame_--;
				camera_.transform_.translate.x = std::cos(frameCount_ * 0.1f) * cameraShakePower_;
				if (cameraShakePower_ > 0.0f){
					cameraShakePower_ -= 0.01f;
				}
			} else {
				isRequestedExit_ = true;
			}
		}
	};

	mainTitlemodel_.transform_ = mainTitleTransform_;
	subTitleModel_.transform_ = subTitleTransform_;

	ice_.transform_.rotate.x = (std::sin(frameCount_ * 0.1f) * 0.1f);
	ice_.transform_.rotate.y += 0.02f;
	Eas::SimpleEaseIn(&ice_.transform_.scale.x, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&ice_.transform_.scale.y, 1.0f, 0.1f);

	azarasi_.Update();

	mainTitlemodel_.Update();
	subTitleModel_.Update();
	ice_.Update();
	camera_.Update();
}

void TitleScene::Draw() {
	ImGui::Begin("TitleScene");
	ImGui::DragFloat3("camera", &camera_.transform_.translate.x, 0.1f);
	ImGui::End();

	mainTitlemodel_.Draw(&camera_);
	subTitleModel_.Draw(&camera_);
	ice_.Draw(&camera_);
	azarasi_.Draw(&camera_);
}

IScene* TitleScene::GetNextScene() { return new GameScene(engineCore_); }
