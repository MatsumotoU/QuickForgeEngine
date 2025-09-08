#include "TitleScene.h"
#include "GameScene.h"


TitleScene::TitleScene(EngineCore* engineCore) :debugCamera_(engineCore) {
	engineCore_ = engineCore;
	engineCore_->GetGraphRenderer()->SetCamera(&camera_);

	sceneID_ = 0;
}

TitleScene::~TitleScene() {
	engineCore_->GetGraphRenderer()->DeleteCamera(&camera_);
}

void TitleScene::Initialize() {
#ifdef _DEBUG
	isActiveDebugCamera_ = false;
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG

	isRequestedExit_ = false;
	camera_.Initialize(engineCore_->GetWinApp());
	camera_.transform_.rotate;
	camera_.transform_.translate = { 1.4f, 0.0f, 1.0f };
	camera_.transform_.rotate = { 0.0f,0.2f,0.0f };
	cameraWorkFream_ = -1.0f;
	cameraWorlEnd = false;

	mole_ = std::make_unique<Mole>();
	mole_.get()->Initialize(engineCore_, &camera_, directionalLightDir_);

	diggingEffect_ = std::make_unique<DiggingEffect>();
	diggingEffect_.get()->Initialize(engineCore_, &camera_, directionalLightDir_);

	wall_ = std::make_unique<Wall>();
	wall_.get()->Initialize(engineCore_, &camera_, directionalLightDir_);

	signBoard_ = std::make_unique<SignBoard>();
	signBoard_.get()->Initialize(engineCore_, &camera_, directionalLightDir_);

	titleName_ = std::make_unique<TitleName>();
	titleName_.get()->Initialize(engineCore_, &camera_, directionalLightDir_);

	titleGround_ = std::make_unique<TitleGround>();
	titleGround_.get()->Initialize(engineCore_, &camera_, directionalLightDir_);

	titleUI_ = std::make_unique<TitleUI>();
	titleUI_.get()->Initialize(engineCore_, &camera_, directionalLightDir_);

	skyDome_ = std::make_unique<TitleSkyDome>();
	skyDome_.get()->Initialize(engineCore_, &camera_);

	
}

void TitleScene::Update() {
	DirectInputManager* input = engineCore_->GetInputManager();
	frameCount_++;

	CameraWork();

	if (cameraWorlEnd) {
		mole_.get()->Update();
	}

	diggingEffect_.get()->Update(mole_.get());

	wall_.get()->Update();

	signBoard_.get()->Update();

	titleName_.get()->Update();

	titleGround_.get()->Update();

	titleUI_.get()->Update(mole_.get());

	skyDome_.get()->Update();


	if (mole_.get()->IsGameStart()) {
		isRequestedExit_ = true;
	}


#ifdef _DEBUG
	if (input->keyboard_.GetTrigger(DIK_P)) {
		isActiveDebugCamera_ = !isActiveDebugCamera_;
	}
	CameraUpdate();
#endif // _DEBUG
	camera_.Update();
}

void TitleScene::Draw() {
	engineCore_->GetGraphRenderer()->DrawGrid();

	mole_.get()->Draw();
	diggingEffect_.get()->Draw();
	wall_.get()->Draw();
	signBoard_.get()->Draw();
	titleName_.get()->Draw();
	titleGround_.get()->Draw();
	titleUI_.get()->Draw(mole_.get());
	skyDome_.get()->Draw();

#ifdef _DEBUG
	debugCamera_.DrawImGui();
	mole_.get()->DebugImGui();
	diggingEffect_.get()->DebugImGui();
	wall_.get()->DebugImGui();
	signBoard_.get()->DebugImGui();
	titleName_.get()->DebugImGui();
	titleGround_.get()->DebugImGui();
	titleUI_.get()->DebugImGui();
	skyDome_.get()->DebugImGui();

	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::DragFloat3("CameraT", &camera_.transform_.translate.x, 0.1f);
		ImGui::DragFloat3("CameraR", &camera_.transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("CameraS", &camera_.transform_.scale.x, 0.1f);
	}
#endif // _DEBUG
}

IScene* TitleScene::GetNextScene() { return new GameScene(engineCore_); }

void TitleScene::CameraUpdate() {
#ifdef _DEBUG
	if (isActiveDebugCamera_) {
		debugCamera_.Update();
		camera_ = debugCamera_.camera_;
	}
#endif // _DEBUG
}

void TitleScene::CameraWork()
{
	Transform start;
	Transform end;

	start.translate = { 1.4f,0.0f,1.0f };
	start.rotate = { 0.0f,0.2f,0.0f };
	end.translate = { 0.0f,0.0f,-5.0f };

	if (cameraWorkFream_ > 0) {
		camera_.transform_.translate.x = (1.0f - cameraWorkFream_) * start.translate.x + cameraWorkFream_ * end.translate.x;
		camera_.transform_.translate.z = (1.0f - cameraWorkFream_) * start.translate.z + cameraWorkFream_ * end.translate.z;
		camera_.transform_.rotate.y = (1.0f - cameraWorkFream_) * start.rotate.y + cameraWorkFream_ * end.rotate.y;
	}
	cameraWorkFream_ += cameraWorkSpeed_;

	if (cameraWorkFream_ >= 1.0f) {
		cameraWorkFream_ = 1.0f;
		cameraWorlEnd = true;
	}

}
