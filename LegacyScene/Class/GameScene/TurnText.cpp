#include "TurnText.h"
#include "Utility/MyEasing.h"

void TurnText::Initialize(EngineCore* engineCore, Camera* camera) {
	playerTurnModel_ = std::make_unique<Model>(engineCore,camera);
	enemyTurnModel_ = std::make_unique<Model>(engineCore, camera);
	playerTurnModel_->LoadModel("Resources/Model/UI", "PlayerTurnobj.obj", COORDINATESYSTEM_HAND_RIGHT);
	enemyTurnModel_->LoadModel("Resources/Model/UI", "EnemyTurnobj.obj", COORDINATESYSTEM_HAND_RIGHT);

	isPlayerTurn_ = true;
	requestTurn_ = true;
	isChanging_ = false;
	isHidden_ = false;

	engineCore_ = engineCore;

	transform_.translate = { 4.0f,6.6f,3.9f };
	transform_.rotate = { 2.0f,0.0f,3.14f };

	color_ = { 1.0f,1.0f,1.0f,1.0f };
}

void TurnText::Update() {
	if (isHidden_) {
		MyEasing::SimpleEaseIn(&transform_.scale.x, 0.0f, 0.3f);
		MyEasing::SimpleEaseIn(&transform_.scale.y, 0.0f, 0.3f);
	}

	if (isPlayerTurn_) {
		MyEasing::SimpleEaseIn(&color_.x, 0.0f, 0.1f);
		MyEasing::SimpleEaseIn(&color_.y, 1.0f, 0.1f);
		MyEasing::SimpleEaseIn(&color_.z, 0.0f, 0.1f);
	} else {
		MyEasing::SimpleEaseIn(&color_.x, 1.0f, 0.1f);
		MyEasing::SimpleEaseIn(&color_.y, 0.0f, 0.1f);
		MyEasing::SimpleEaseIn(&color_.z, 1.0f, 0.1f);
	}

	if (isChanging_) {
		MyEasing::SimpleEaseIn(&transform_.scale.x, 0.0f, 0.3f);
		MyEasing::SimpleEaseIn(&transform_.scale.y, 1.5f, 0.3f);

		if (transform_.scale.x < 0.01f) {
			isChanging_ = false;
			isPlayerTurn_ = requestTurn_;
		}
	} else {
		MyEasing::SimpleEaseIn(&transform_.scale.x, 1.0f, 0.3f);
		MyEasing::SimpleEaseIn(&transform_.scale.y, 1.0f, 0.3f);
	}

	playerTurnModel_->transform_ = transform_;
	enemyTurnModel_->transform_ = transform_;

	playerTurnModel_->SetColor(color_);
	enemyTurnModel_->SetColor(color_);

	playerTurnModel_->Update();
	enemyTurnModel_->Update();
}

void TurnText::Draw() {
#ifdef _DEBUG
	ImGui::Begin("TurnText");
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
	ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
	ImGui::End();
#endif // _DEBUG

	if (isHidden_) {
		if (transform_.scale.Length() < 0.1f) {
			return;
		}
	}

	if (isPlayerTurn_) {
		playerTurnModel_->Draw();
	} else {
		enemyTurnModel_->Draw();
	}
}

void TurnText::ChangeTurn(bool set) {
	requestTurn_ = set;
	isChanging_ = true;
}