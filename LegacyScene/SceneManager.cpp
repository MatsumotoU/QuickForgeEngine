#include "SceneManager.h"

SceneManager::SceneManager(EngineCore* engineCore) :fade_(engineCore){
	engineCore_ = engineCore;
	fade_.Initialize();

	//scene = new GameScene(engineCore_);
	scene = new StageSelectScene(engineCore_);
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

	/*ImGui::Begin("FPS");
	ImGui::Text("fps = %f", engineCore_->GetFpsCounter()->GetFps());
	ImGui::Text("averageFps = %f", engineCore_->GetFpsCounter()->GetAverageFps());
	
	if (ImPlot::BeginPlot("Fps Plots")) {
		float x[64];
		for (float i = 0.0f; i < engineCore_->GetFpsCounter()->GetFpsSamplerNum(); i++) {
			x[static_cast<int>(i)] = i;
		}

		ImPlot::SetupAxes("x", "y");
		ImPlot::PlotLine("f(x)", x, engineCore_->GetFpsCounter()->GetFpsSample(), engineCore_->GetFpsCounter()->GetFpsSamplerNum());
		ImPlot::EndPlot();
	}

	ImGuiKnobs::Knob("Volume", &value_, -6.0f, 6.0f, 0.1f, "%.1fdB", ImGuiKnobVariant_Tick);

	ImGui::End();*/
}
