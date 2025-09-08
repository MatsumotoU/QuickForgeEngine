#pragma once
#include "Base/EngineCore.h"

class Mole;

class TitleUI
{
public:
	TitleUI() = default;
	~TitleUI() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir);
	void Update(Mole* mole);
	void Draw(Mole* mole);

	void UpdateStickHold(Mole* mole);

	void ArrowSizeUP();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
	std::unique_ptr<Model> arrowModel_;
	std::unique_ptr<Model> mouseModel_;

	float rotetoMax = 3.14f / 4.0f;
	float roteta_ = 0;
	float rotetaSpeed_ = 3.0f;

	int colorFream_ = 0;
	bool isRed_;

	enum ArrowDirection {
		None,
		Left,
		Right,
	};
	ArrowDirection arrowDirection_ = None;
	bool isArrow_ = false;
	bool arrowSizeUPFlag_;
	Vector3 arrowSizeUPSpeed_ = { 0.2f,0.2f,0.2f };

	Vector2 mouseTargetPos;
	Vector2 input_;

	Vector3 directionalLightDir_;


};

