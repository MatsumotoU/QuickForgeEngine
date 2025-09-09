#include "DiggingEffect.h"

#include "Mole.h"


void DiggingEffect::DebugImGui() {
    if (ImGui::CollapsingHeader("Digging Effect")) {
        // パーティクル最大数
        ImGui::DragInt("Particle Max", (int*)&particleMax_, 1, 1, 100);

        // 共通の速度
        ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f);

        // 各パーティクルの情報を表示
        for (int i = 0; i < particle_.size(); i++) {
            std::string label = "Particle " + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                ImGui::DragFloat("LifeTime", &particle_[i].lefeTime, 0.1f, 0.0f, 100.0f);

                // モデルがあれば transform をいじる
                if (particle_[i].model) {
                    ImGui::DragFloat3("Translate", &particle_[i].model->transform_.translate.x, 0.1f);
                    ImGui::DragFloat3("Scale", &particle_[i].model->transform_.scale.x, 0.1f);
                    ImGui::DragFloat3("Rotate", &particle_[i].model->transform_.rotate.x, 0.1f);
                }

                ImGui::TreePop();
            }
        }
    }
}


void DiggingEffect::Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir) {
	engineCore_ = engineCore;
	camera_ = camera;
    directionalLightDir_ = directionalLightDir;

	for (int i = 0; i < particleMax_; i++) {
		Particle newParticle;
		newParticle.model = std::make_unique<Model>(engineCore_, camera_);
		newParticle.model->LoadModel("Resources/Model/wall/soil", "soil.obj", COORDINATESYSTEM_HAND_LEFT);
		newParticle.model->transform_.scale = { 0.8f,0.8f,0.8f };
		newParticle.model->SetDirectionalLightDir(directionalLightDir_);
		particle_.push_back(std::move(newParticle));
	}
}

void DiggingEffect::Update(Mole* mole) {
    if (mole->GetMoleState() == Mole::MoleState::Digging) {
        SpwnParticle(mole);
        ParticleUpdate();
    }
    for (int i = 0; i < particle_.size(); i++) {
        particle_[i].lefeTime--;
        if (particle_[i].lefeTime < 0) {
            particle_[i].lefeTime = 0;
        }
    }
}

void DiggingEffect::Draw() {
	for (int i = 0; i < particle_.size(); i++) {
        if (particle_[i].lefeTime > 0) {
            particle_[i].model.get()->Draw();
        }
	}
}

void DiggingEffect::SpwnParticle(Mole* mole)  
{  
	for (int i = 0; i < particle_.size(); i++) {
		if (particle_[i].lefeTime <= 0) {
			particle_[i].lefeTime = 30;
			float offsetX = (rand() % 100 - 50) / 100.0f; // -0.5f ~ 0.5f  
			float offsetZ = (rand() % 100 - 50) / 100.0f;
			particle_[i].model->transform_.translate = mole->GetTranslate() + Vector3(offsetX, 0.0f, offsetZ);
			particle_[i].model->transform_.scale = { 0.8f,0.8f,0.8f };
			return;
		}
	} 
}

void DiggingEffect::ParticleUpdate()
{
    for (int i = 0; i < particle_.size(); i++) {
        if (particle_[i].lefeTime > 0) {
            particle_[i].model.get()->transform_.translate += velocity_;
        }
        particle_[i].model.get()->Update();
    }
}

