#include "ResultUI.h"

void ResultUI::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;
	niceTextModel_ = std::make_unique<Model>(engineCore, camera);
	retryTextModel_ = std::make_unique<Model>(engineCore, camera);
	toSelectTextModel_ = std::make_unique<Model>(engineCore, camera);
	toNextStageTextModel_ = std::make_unique<Model>(engineCore, camera);

	niceTextModel_->LoadModel("Resources/Model/UI", "NiceMole.obj", COORDINATESYSTEM_HAND_RIGHT);
	retryTextModel_->LoadModel("Resources/Model/UI", "Retry.obj", COORDINATESYSTEM_HAND_RIGHT);
	toSelectTextModel_->LoadModel("Resources/Model/UI", "StageSelect.obj", COORDINATESYSTEM_HAND_RIGHT);
	toNextStageTextModel_->LoadModel("Resources/Model/UI", "NextStage.obj", COORDINATESYSTEM_HAND_RIGHT);

	niceTextModel_->SetColor({ 1.0f,0.5f,0.5f,1.0f });

	transform_.translate = { 5.2f,21.0f,-23.0f };
	transform_.rotate.y = -0.5f;

	topOffset_ = 2.3f;
	bottomOffset_ = 4.2f;
	isCleard_ = false;
	selectedTop_ = true;

	selectSE_ = engineCore_->LoadSoundData("Resources/Sound/SE/", "ToTitle.mp3");
}

void ResultUI::Update() {
	float deltaTime = engineCore_->GetDeltaTime();
	time_ += deltaTime;

	if (selectInterval_ > 0.0f) {
		selectInterval_ -= deltaTime;
		if (selectInterval_ < 0.0f) {
			selectInterval_ = 0.0f;
		}
	}

	niceTextModel_->transform_ = transform_;
	retryTextModel_->transform_ = transform_;
	toSelectTextModel_->transform_ = transform_;
	toNextStageTextModel_->transform_ = transform_;

	retryTextModel_->transform_.translate.y -= topOffset_;
	toNextStageTextModel_->transform_.translate.y -= topOffset_;
	toSelectTextModel_->transform_.translate.y -= bottomOffset_;

	niceTextModel_->transform_.scale.x = 1.5f + (sinf(time_) * 0.2f);
	niceTextModel_->transform_.scale.y = 1.5f + (cosf(time_) * 0.2f);

	if (selectedTop_) {
		retryTextModel_->transform_.scale.x = 1.2f + (cosf(time_*2.0f) * 0.2f);
		retryTextModel_->transform_.scale.y = 1.2f + (sinf(time_ * 2.0f) * 0.2f);
		toNextStageTextModel_->transform_.scale.x = 1.2f + (cosf(time_ * 2.0f) * 0.2f);
		toNextStageTextModel_->transform_.scale.y = 1.2f + (cosf(time_ * 2.0f) * 0.2f);

		toSelectTextModel_->transform_.scale.x = 1.0f;
		toSelectTextModel_->transform_.scale.y = 1.0f;

	} else {
		toSelectTextModel_->transform_.scale.x = 1.2f + (cosf(time_ * 2.0f) * 0.2f);
		toSelectTextModel_->transform_.scale.y = 1.2f + (sinf(time_ * 2.0f) * 0.2f);

		retryTextModel_->transform_.scale.x = 1.0f;
		retryTextModel_->transform_.scale.y = 1.0f;
		toNextStageTextModel_->transform_.scale.x = 1.0f;
		toNextStageTextModel_->transform_.scale.y = 1.0f;

	}

	if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_UP) || engineCore_->GetXInputController()->GetLeftStick(0).y > 3.0f) {
		if (selectInterval_ <= 0.0f) {
			selectedTop_ = !selectedTop_;
			selectInterval_ = 0.2f;
			engineCore_->GetAudioPlayer()->PlayAudio(selectSE_, "ToTitle.mp3", false);
		}
	}
	if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_DOWN) || engineCore_->GetXInputController()->GetLeftStick(0).y < -3.0f) {
		if (selectInterval_ <= 0.0f) {
			selectedTop_ = !selectedTop_;
			selectInterval_ = 0.2f;
			engineCore_->GetAudioPlayer()->PlayAudio(selectSE_, "ToTitle.mp3", false);
		}
	}

	niceTextModel_->Update();
	retryTextModel_->Update();
	toSelectTextModel_->Update();
	toNextStageTextModel_->Update();

}

void ResultUI::Draw() {
#ifdef _DEBUG
	ImGui::Begin("ResultUI");

	ImGui::DragFloat3("ResultUI Position", &transform_.translate.x, 0.1f);
	ImGui::DragFloat("ResultUI Top Offset", &topOffset_, 0.1f);
	ImGui::DragFloat("ResultUI Bottom Offset", &bottomOffset_, 0.1f);

	ImGui::End();
#endif // _DEBUG

	niceTextModel_->Draw();
	if (isCleard_)
		toNextStageTextModel_->Draw();
	else {
		retryTextModel_->Draw();
	}
	toSelectTextModel_->Draw();
}
