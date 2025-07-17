#include "GameScene.h"
#include "TitleScene.h"

GameScene::GameScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();
}

GameScene::~GameScene() {
	engineCore_->GetGraphRenderer()->DeleteCamera();
}

void GameScene::Initialize() {
	isInitialized_ = true;
#ifdef _DEBUG
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -30.0f;
#endif // _DEBUG
	camera.Initialize(engineCore_->GetWinApp());
	camera.transform_.translate.z = -30.0f;

	/*player.Initialize(engineCore_);
	player.SetMap(&map_);
	player.transform_.translate = map_.GetMapChipPositionByIndex(1, 18);
	skyDome_.Initialize(engineCore_);
	map_.Initialize(engineCore_);

	cameraContoroller_.Initialize(&camera);
	cameraContoroller_.SetTargetPosition(&player.transform_.translate);
	cameraContoroller_.SetTargetVelocity(&player.velocity_);

	enemy_.Initialize(engineCore_);
	enemy_.transform_.translate = map_.GetMapChipPositionByIndex(15, 18);

	deathParticle_.Initialize(engineCore_);*/
	isRequestedExit_ = false;
}

void GameScene::Update() {
#ifdef _DEBUG
	debugCamera_.Update();
	camera = debugCamera_.camera_;
#endif // _DEBUG
	camera.Update();
	/*cameraContoroller_.Update();
	player.Update();
	enemy_.Update();

	if ((player.transform_.translate - enemy_.transform_.translate).Length() <= kWith) {
		if (player.GetIsActive()) {
			player.SetIsActive(false);
			deathParticle_.EmmitParticle(player.transform_.translate);
		}
	}

	if (!player.GetIsActive()) {
		if (!deathParticle_.GetIsActive()) {
			isRequestedExit_ = true;
		}
	}*/

	if (input_->keyboard_.GetTrigger(DIK_ESCAPE)) {
		isRequestedExit_ = true;
	}

	//deathParticle_.Update();
}

void GameScene::Draw() {
#ifdef _DEBUG
	assert(engineCore_);
#endif // _DEBUG
	/*map_.Draw(&camera);
	skyDome_.Draw(&camera);
	player.Draw(&camera);
	enemy_.Draw(&camera);
	deathParticle_.Draw(&camera);*/
}

std::unique_ptr<IScene> GameScene::GetNextScene() {
	return std::make_unique<TitleScene>(engineCore_);
}
