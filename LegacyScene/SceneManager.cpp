#include "SceneManager.h"

SceneManager::SceneManager(EngineCore* engineCore) :fade_(engineCore){
	engineCore_ = engineCore;
	fade_.Initialize();

	//scene = new GameScene(engineCore_, &json_);
	scene = new TitleScene(engineCore_,&json_);
	//scene = new StageSelectScene(engineCore_, &json_);
}

SceneManager::~SceneManager() {
	delete scene;
}

void SceneManager::Initialize() {
	scene->Initialize();
}

void SceneManager::Update() {
	if (scene->GetReqesytedExit()) {

		if (fade_.isEndFadeOut_) {
			IScene* nextScene = scene->GetNextScene();
			delete scene;
			scene = nextScene;
			scene->Initialize();
			fade_.StartFadeIn();
			return;
		}

		if (!fade_.isFadeOut_) {
			fade_.StartFadeOut();
		}
	}
	fade_.Update();

	scene->Update();
}

void SceneManager::Draw() {
	scene->Draw();
	if (scene->GetReqesytedExit() || fade_.isFading()) {
		fade_.Draw();
	}
}
