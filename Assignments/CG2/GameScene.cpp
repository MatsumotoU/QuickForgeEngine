#include "GameScene.h"

GameScene::GameScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();
}

void GameScene::Initialize() {
	for (int i = 0; i < kParticles; i++) {
		model[i].Initialize(engineCore_->GetDirectXCommon(), engineCore_->GetTextureManager(), engineCore_->GetGraphicsCommon()->GetTrianglePso());
		model[i].LoadModel("Resources", "Triangle.obj", COORDINATESYSTEM_HAND_RIGHT);
	}

	skyDome_.Initialize(engineCore_->GetDirectXCommon(), engineCore_->GetTextureManager(), engineCore_->GetGraphicsCommon()->GetTrianglePso());
	skyDome_.LoadModel("Resources", "skyDome.obj", COORDINATESYSTEM_HAND_RIGHT);
	skyDome_.material_.materialData_->enableLighting = false;

#ifdef _DEBUG
	debugCamera_.Initialize(engineCore_->GetWinApp(), engineCore_->GetInputManager());
#endif // _DEBUG

	camera.Initialize(engineCore_->GetWinApp());
	t = 0.0f;
	for (int i = 0; i < kParticles; i++) {
		float c = (3.14f * 2.0f) * (static_cast<float>(i) / static_cast<float>(kParticles));
		particles[i].translate.x =  cosf(c);
		particles[i].translate.z = sinf(c);
	}

	color = { 1.0f,1.0f,1.0f,1.0f };
	engineCore_->GetOffscreen()->SetPSO(engineCore_->GetGraphicsCommon()->GetGrayScalePso());
}

void GameScene::Update() {
#ifdef _DEBUG
	debugCamera_.Update();
	camera = debugCamera_.camera_;
#endif // _DEBUG
	t += 0.01f;
	for (int i = 0; i < kParticles; i++) {
		float c = (3.14f * 2.0f) * (static_cast<float>(i) / static_cast<float>(kParticles));
		particles[i].translate.x = cosf(c + t * 2.0f);
		particles[i].translate.z = sinf(c+t);

		particles[i].rotate.x = t + c;
		particles[i].rotate.y = -t + c;
		particles[i].rotate.z = t + c;

		Vector4 color = { fabsf(sin(c+t)),fabsf(cos(c + t)) ,fabsf(sin(t)),1.0f};
		Vector4 defaultColor = { 1.0f,1.0f,1.0f,1.0f };

		color = Vector4::Leap(color, defaultColor, fabsf(sin(t)));

		model[i].material_.materialData_->color = color;
	}
	
}

void GameScene::Draw() {
#ifdef _DEBUG
	//model.Draw(transform,&debugCamera_.camera_);

	ImGui::Begin("offscreen");
	ImGui::ColorPicker4("Color", &color.x);
	engineCore_->GetOffscreen()->material_.materialData_->color = color;

	if (ImGui::Button("defaultSader")) {
		engineCore_->GetOffscreen()->SetPSO(engineCore_->GetGraphicsCommon()->GetTrianglePso());
	}

	if (ImGui::Button("GrayscaleSader")) {
		engineCore_->GetOffscreen()->SetPSO(engineCore_->GetGraphicsCommon()->GetGrayScalePso());
	}

	ImGui::End();
#endif // _DEBUG

	skyDome_.Draw(transform, &camera);
	
	for (int i = 0; i < kParticles; i++) {
		model[i].Draw(particles[i], &camera);
	}
}
