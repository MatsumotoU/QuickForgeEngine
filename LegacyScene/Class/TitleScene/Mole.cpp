#include "Mole.h"

void Mole::DebugImGui()
{
	if (ImGui::CollapsingHeader("Mole")) {
		if (ImGui::CollapsingHeader("Transform_")) {
			ImGui::DragFloat3("MoleTranslate", &model_.get()->transform_.translate.x, 0.1f);
			ImGui::DragFloat3("MoleRotate", &model_.get()->transform_.rotate.x, 0.1f);
			ImGui::DragFloat3("MoleScale", &model_.get()->transform_.scale.x, 0.1f);
		}
		if (ImGui::CollapsingHeader("Animmation")) {
			ImGui::Checkbox("IsAnimation", &isAnimation_);
			ImGui::DragFloat("AnimationTimer", &animationTimer_);
			int stateIndex = static_cast<int>(moleState_);
			if (ImGui::Combo("Mole State", &stateIndex, stateNames, IM_ARRAYSIZE(stateNames))) {
				// int → enum に変換
				moleState_ = static_cast<MoleState>(stateIndex);
			}
		}

	}
}

void Mole::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;
	isGameStart = false;
	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/mole", "mole.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { -1.7f,-1.0f,2.0f };
	model_.get()->transform_.rotate = { 0.0f,1.8f,0.0f };
	model_.get()->transform_.scale = { 1.0f,1.0f,1.0f };

}

void Mole::Update() {

	Vector2 leftStick = engineCore_->GetXInputController()->GetLeftStick(0);

	if (leftStick.x != 0 &&
		moleState_ == MoleState::Normal) {
		isAnimation_ = true;
		if (leftStick.x < 0) {
			direction_ = Direction::Left;
		}
		else {
			direction_ = Direction::Right;
		}
	}

	Animation();
	Move();

	model_->Update();
}

void Mole::Draw() {
	model_->Draw();
}

void Mole::Animation()
{
	float t = (float)animationTimer_ / animationDuration_; // 0.0～1.0

	float scale = 1.0f;
	switch (moleState_)
	{
	case Mole::MoleState::Normal:
		if (isAnimation_) {
			moleState_ = MoleState::StartDigging;
		}
		break;
	case Mole::MoleState::StartDigging:
		animationTimer_++;

		if (t < 0.5f) {
			// 前半：だんだん大きくなる
			scale = 1.0f + t * 0.5f * 2.0f; // 1.0 → 1.5

		}
		else {
			// 後半：だんだん小さくなる
			float nt = (t - 0.5f) * 2.0f; // 0～1
			scale = 1.5f - nt * 1.5f;  // 1.5 → 1.0
		}

		if (animationTimer_ >= animationDuration_) {
			moleState_ = Mole::MoleState::Digging;
			animationTimer_ = 0; // 次の準備
			scale = 0.0f;       // 完全に消える
		}

		model_.get()->transform_.scale = { scale, scale, scale };
		break;
	case Mole::MoleState::Digging:
		if (!isAnimation_) {
			animationTimer_ = 0;
			moleState_ = Mole::MoleState::Emerging;
		}
		break;
	case Mole::MoleState::Emerging:
		animationTimer_++;

		if (t < 0.5f) {
			// 前半：小さい状態から大きくなる
			scale = 0.0f + t * 1.5f * 2.0f; // 0.0 → 1.5
		}
		else {
			// 後半：1.5から1.0に戻る
			float nt = (t - 0.5f) * 2.0f; // 0～1
			scale = 1.5f - nt * 0.5f; // 1.5 → 1.0
		}

		if (animationTimer_ >= animationDuration_) {
			moleState_ = Mole::MoleState::Normal;
			animationTimer_ = 0;
			scale = 1.0f; // 元サイズに戻す
		}

		model_.get()->transform_.scale = { scale, scale, scale };

		break;

	default:
		break;
	}


}

void Mole::Move()
{
	
	if (moleState_ == MoleState::Digging) {
		moveTimer_++;
		if (direction_ == Direction::Left) {
			model_.get()->transform_.translate.x += moveVelocity_ / 60.0f;
		}
		else {
			model_.get()->transform_.translate.x -= moveVelocity_ / 60.0f;
		}
		ClampPosition();
		if (moveTimer_ >= moveMaxTime_) {
			moveTimer_ = 0;
			isAnimation_ = false;
		}
	}
}

void Mole::ClampPosition()
{
	float posX = model_.get()->transform_.translate.x;
	if (posX < -2.8f) {
		model_.get()->transform_.translate.x = -2.7f;
		moveTimer_ = float(moveMaxTime_);
	}
	if (posX > 2.8f) {
		isGameStart = true;
	}
}
