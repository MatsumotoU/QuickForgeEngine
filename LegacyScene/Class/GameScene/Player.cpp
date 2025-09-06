#include "Player.h"

void Player::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;
	model_ = std::make_unique<Model>(engineCore, camera);
	model_->LoadModel("Resources", "Cube.obj", COORDINATESYSTEM_HAND_RIGHT);

	accelerationDamping_ = 0.95f;
	velocityDamping_ = 0.98f;

	isAlive_ = true;
	isCanMove_ = true;
	isCanShot_ = false;
	isMoving_ = false;
	isReqestBuilding_ = false;
	isGrounded_ = false;

	moveTimer_ = 0.0f;
	maxMoveTimer_ = 1.0f;
	shotPower_ = 6.0f;
	moveDir_ = { 0.0f,0.0f };

	model_->transform_.translate.y = 1.0f;
	alpha_ = 1.0f;
	model_->SetColor({ 0.0f,1.0f,0.0f,alpha_ });
}

void Player::Update() {
	float deltaTime = engineCore_->GetDeltaTime();
	// はじきの処理
	if (isCanMove_) {
		// まだ動いてないときにスティックを倒したら動けるようにする
		Vector2 leftStick = engineCore_->GetXInputController()->GetLeftStick(0);
		if (leftStick.Length() >= 3.0f) {
			isCanShot_ = true;
			moveDir_ = -leftStick.Normalize();
			moveTimer_ = maxMoveTimer_;
		} else {
			if (isCanShot_) {
				isCanMove_ = false;
				isCanShot_ = false;
				isMoving_ = true;
			}
		}

		// ボタンでも発射できる
		if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_SPACE) || engineCore_->GetXInputController()->GetTriggerButton(XINPUT_GAMEPAD_A, 0)) {
			if (isCanShot_) {
				moveDir_ = -leftStick.Normalize();
				isCanMove_ = false;
				isCanShot_ = false;
				isMoving_ = true;
			}
		}
	}

	// 動いてる最中の処理
	if (isMoving_) {
		if (moveTimer_ > 0.0f) {
			model_->transform_.translate.x += moveDir_.x * shotPower_ * deltaTime;
			model_->transform_.translate.z += moveDir_.y * shotPower_ * deltaTime;
			moveTimer_ -= deltaTime;
		} else {
			isMoving_ = false;
			isReqestBuilding_ = true;
		}
	}

	// 移動の処理
	model_->transform_.translate += velocity_ * deltaTime;
	velocity_ = velocity_ * velocityDamping_;

	if (velocity_.y > -5.0f) {
		velocity_.y -= 9.81f * deltaTime;
	}

	if (model_->transform_.translate.y <= 1.0f) {
		model_->transform_.translate.y = 1.0f;
		velocity_.y = 0.0f;

		if (isJumping_) {
			isJumping_ = false;
			isGrounded_ = true;
		}
	}

	model_->SetColor({ 0.0f,1.0f,0.0f,alpha_ });
	model_->Update();
}

void Player::Draw() {
	ImGui::Begin("Player");

	ImGui::DragFloat3("Velocity", &velocity_.x, 0.1f);
	ImGui::DragFloat3("Position", &model_->transform_.translate.x, 0.1f);
	ImGui::DragFloat("ShotPower", &shotPower_, 0.01f, 0.0f, 10.0f);
	ImGui::Checkbox("isCanMove", &isCanMove_);
	ImGui::Checkbox("isCanShot", &isCanShot_);
	ImGui::Checkbox("isMoving", &isMoving_);
	ImGui::Checkbox("isReqestBuilding", &isReqestBuilding_);
	ImGui::Text("MoveDir:%f,%f", moveDir_.x, moveDir_.y);
	ImGui::Text("MoveTimer:%f", moveTimer_);

	Vector2 leftStick = engineCore_->GetXInputController()->GetLeftStick(0);
	ImGui::Text("%f,%f", leftStick.x, leftStick.y);

	ImGui::End();

	model_->Draw();
}