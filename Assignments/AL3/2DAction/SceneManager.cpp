#include "SceneManager.h"

SceneManager::SceneManager(EngineCore* engineCore) {
	engineCore_ = engineCore;
	//scene = new GameScene(engineCore_);
	fade_.Initialize(engineCore_);
	scene = std::make_unique<TitleScene>(engineCore_);
}

SceneManager::~SceneManager() {
}

void SceneManager::Initialize() {
	scene->Initialize();
}

void SceneManager::Update() {
	if (scene->GetReqesytedExit()) {

		if (fade_.isEndFadeOut_) {
			DebugLog("Transition Scene");
			std::unique_ptr<IScene> nextScene = scene->GetNextScene();
			assert(nextScene);
			assert(scene);
			scene = std::move(nextScene);

			scene.get()->Initialize();
			scene.get()->Update();
			fade_.StartFadeIn();
			return;
		}

		if (!fade_.isFadeOut_) {
			fade_.StartFadeOut();
		}
	} else {
		scene->Update();
	}
	fade_.Update();
}

void SceneManager::Draw() {
	
	assert(scene.get());
	if (scene.get()->isInitialized_) {
		scene->Draw();
	}
	
	if (scene->GetReqesytedExit() || fade_.isFading()) {
		fade_.Draw();
	}
}
