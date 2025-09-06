#include "TitleGround.h"

void TitleGround::DebugImGui()
{
	if (ImGui::CollapsingHeader("TitleGround")) {
		for (int i = 0; i < vertical; i++) {
			for (int j = 0; j < horizontal; j++) {
				std::string header = "Model (" + std::to_string(i) + "," + std::to_string(j) + ")";
				if (ImGui::CollapsingHeader(header.c_str())) {
					// Translate
					std::string labelT = "Translate###Translate_" + std::to_string(i) + "_" + std::to_string(j);
					ImGui::DragFloat3(labelT.c_str(), &model_[i][j]->transform_.translate.x, 0.1f);

					// Rotate
					std::string labelR = "Rotate###Rotate_" + std::to_string(i) + "_" + std::to_string(j);
					ImGui::DragFloat3(labelR.c_str(), &model_[i][j]->transform_.rotate.x, 0.1f);

					// Scale
					std::string labelS = "Scale###Scale_" + std::to_string(i) + "_" + std::to_string(j);
					ImGui::DragFloat3(labelS.c_str(), &model_[i][j]->transform_.scale.x, 0.1f);
				}
			}
		}
	}
}

void TitleGround::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	for (int i = 0; i < vertical; i++) {
		std::vector<std::unique_ptr<Model>> row;
		for (int j = 0; j < horizontal; j++) {
			std::unique_ptr <Model> newmodel = std::make_unique<Model>(engineCore_, camera_);
			newmodel->LoadModel("Resources/Model/blocks/grass", "grass.obj", COORDINATESYSTEM_HAND_LEFT);
			newmodel.get()->transform_.translate = { j * blockSize - (horizontal / 2 * blockSize),-1.5f, i * blockSize };
			row.push_back(std::move(newmodel));
		}
		model_.push_back(std::move(row));
	}
}

void TitleGround::Update() {
	for (int i = 0; i < vertical; i++) {
		for (int j = 0; j < horizontal; j++) {
			model_[i][j]->Update();
		}
	}
}

void TitleGround::Draw() {
	for (int i = 0; i < vertical; i++) {
		for (int j = 0; j < horizontal; j++) {
			model_[i][j]->Draw();
		}
	}
}