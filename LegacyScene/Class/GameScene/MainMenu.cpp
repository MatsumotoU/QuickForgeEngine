#include "MainMenu.h"

void MainMenu::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	menuSelect_ = MenuSelect::ResetGame;

	menuTextModel_ = std::make_unique<Model>(engineCore, camera);
	menuTextModel_->LoadModel("Resources/Model/UI", "Menu.obj", COORDINATESYSTEM_HAND_RIGHT);
	returnSelectTextModel_ = std::make_unique<Model>(engineCore, camera);
	returnSelectTextModel_->LoadModel("Resources/Model/UI", "StageSelect.obj", COORDINATESYSTEM_HAND_RIGHT);
	resetGameTextModel_ = std::make_unique<Model>(engineCore, camera);
	resetGameTextModel_->LoadModel("Resources/Model/UI", "Retry.obj", COORDINATESYSTEM_HAND_RIGHT);

	menuTextModel_->SetColor({ 1.0f,0.5f,0.5f,1.0f });

	menuTextModel_->transform_.rotate.y = -0.5f;
	returnSelectTextModel_->transform_.rotate.y = -0.5f;
	resetGameTextModel_->transform_.rotate.y = -0.5f;

	menuTextModel_->transform_.scale = { 2.0f,2.0f,2.0f };
	transform_.translate = { 6.4f,21.0f,-23.0f };

	Update();

	time_ = 0.0f;
	selectInterval_ = 0.0f;
}

void MainMenu::Update() {
	float deltaTime = engineCore_->GetDeltaTime();
	time_ += deltaTime;

	if (selectInterval_ > 0.0f) {
		selectInterval_ -= deltaTime;
		if (selectInterval_ < 0.0f) {
			selectInterval_ = 0.0f;
		}
	}

	menuTextModel_->transform_.scale.x = 2.0f + (cosf(time_) * 0.2f);
	menuTextModel_->transform_.scale.y = 2.0f + (sinf(time_) * 0.2f);

	if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_UP) || engineCore_->GetXInputController()->GetLeftStick(0).y > 3.0f) {
		if (selectInterval_ <= 0.0f) {
			if (menuSelect_ == MenuSelect::ReturnSelect) {
				menuSelect_ = MenuSelect::ResetGame;
			} else if (menuSelect_ == MenuSelect::ResetGame) {
				menuSelect_ = MenuSelect::ReturnSelect;
			}
			selectInterval_ = 0.2f;
		}
	}
	if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_DOWN) || engineCore_->GetXInputController()->GetLeftStick(0).y < -3.0f) {
		if (selectInterval_ <= 0.0f) {
			if (menuSelect_ == MenuSelect::ReturnSelect) {
				menuSelect_ = MenuSelect::ResetGame;
			} else if (menuSelect_ == MenuSelect::ResetGame) {
				menuSelect_ = MenuSelect::ReturnSelect;
			}
			selectInterval_ = 0.2f;
		}
	}

	if (menuSelect_ == MenuSelect::ReturnSelect) {
		returnSelectTextModel_->transform_.scale.x = 1.0f + (sinf(time_ * 4.0f) * 0.1f);
		returnSelectTextModel_->transform_.scale.y = 1.0f + (cosf(time_ * 4.0f) * 0.1f);

		resetGameTextModel_->transform_.scale.x = 1.0f;
		resetGameTextModel_->transform_.scale.y = 1.0f;
	}

	if (menuSelect_ == MenuSelect::ResetGame) {
		resetGameTextModel_->transform_.scale.x = 1.0f + (sinf(time_ * 4.0f) * 0.1f);
		resetGameTextModel_->transform_.scale.y = 1.0f + (cosf(time_ * 4.0f) * 0.1f);

		returnSelectTextModel_->transform_.scale.x = 1.0f;
		returnSelectTextModel_->transform_.scale.y = 1.0f;
	}

	menuTextModel_->transform_.translate = transform_.translate;
	returnSelectTextModel_->transform_.translate = transform_.translate;
	returnSelectTextModel_->transform_.translate.y -= 5.0f;
	resetGameTextModel_->transform_.translate = transform_.translate;
	resetGameTextModel_->transform_.translate.y -= 3.0f;

	menuTextModel_->Update();
	returnSelectTextModel_->Update();
	resetGameTextModel_->Update();
}

void MainMenu::Draw() {
	menuTextModel_->Draw();
	returnSelectTextModel_->Draw();
	resetGameTextModel_->Draw();

#ifdef _DEBUG
	ImGui::Begin("Menu");
	ImGui::DragFloat3("MenuPos", &transform_.translate.x, 0.1f);
	ImGui::End();
#endif // _DEBUG
}

MenuSelect MainMenu::GetMenuSelect() const {
	return menuSelect_;
}
