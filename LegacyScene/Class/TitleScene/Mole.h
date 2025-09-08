#pragma once
#include "Base/EngineCore.h"

class Mole
{
public:

	enum class MoleState {
		Normal,       // 通常
		StartDigging, // 潜り開始
		Digging,      // 潜っている
		Emerging,     // 出てくる
	};

	enum class Direction {
		Left,   // 左向き
		Right   // 右向き
	};

	Mole() = default;
	~Mole() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir);
	void Update();
	void Draw();

	void Animation();

	void MoveInput();

	void Move();

	void ClampPosition();

	Vector3 GetTranslate() { return model_.get()->transform_.translate; }

	MoleState GetMoleState() { return moleState_; }
	Direction GetDirection() { return direction_; }

	bool IsGameStart() { return isGameStart; }
	bool ISAnimation() { return isAnimation_; }
	bool IsSetMove() { return isSetMove_; }

private:

	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;

	bool isGameStart = false;

	Direction direction_;
	float startDirection_;
	float rotetaDirection_ = 1.8f;
	float rotateFream_ = 0.0f; 

	//アニメション

	const char* stateNames[4] = {
	"Normal",       // 0
	"StartDigging", // 1
	"Digging",      // 2
	"Emerging"      // 3
	};

	MoleState moleState_ = MoleState::Normal;

	bool isAnimation_;
	float animationTimer_;
	int animationDuration_ = 30;//アニメーションの全体フレーム

	//移動用
	float moveTimer_ = 0;
	int moveMaxTime_ = 45;
	float moveVelocity_ = 4.0f;

	Vector2 mouseTargetPos;
	Vector2 input_;
	float deadZone_ = 3.0f;
	bool isSetMove_;
	bool isCleck_;

	//動き
	float rotetoMax = 3.14f / 4.0f;
	float roteta_ = 0;
	float speed_ = 3.0f;

	Vector3 directionalLightDir_;
};

