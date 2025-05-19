#include "SceneManager.h"

SceneManager::SceneManager(EngineCore* engineCore) {
	engineCore_ = engineCore;
	fade_.Initialize(engineCore_);

	scene = new GameScene(engineCore_);
	//scene = new TitleScene(engineCore_);
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

	ImGui::Begin("FPS");
	ImGui::Text("fps = %f", engineCore_->GetFpsCounter()->GetFps());
	ImGui::Text("averageFps = %f", engineCore_->GetFpsCounter()->GetAverageFps());
	ImGui::End();
}
