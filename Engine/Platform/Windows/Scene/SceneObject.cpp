#include "SceneObject.h"
#include "Assets/AssetManager.h"
#include <cassert>

#include "Assets/3DModel/Data/ModelHandle.h"
#include "Assets/Sprite/Data/SpriteData.h"

#include "Renderer/ModelRenderer.h"
#include "Renderer/SpriteRenderer.h"

SceneObject::SceneObject() {
	assetManager_ = nullptr;
	isRequestedExit_ = false;
	sceneName_ = "NewScene";
}

SceneObject::~SceneObject() {
}

void SceneObject::Initialize() {
	assetManager_ = AssetManager::GetInstance();
	assert(assetManager_);
	isRequestedExit_ = false;
	
}

void SceneObject::Update() {
}

void SceneObject::Draw() {
	// モデル描画
	if (assetManager_->GetEntityManager()->HasComponentStrage<ModelHandle>()) {
		const auto& modelStrage = assetManager_->GetEntityManager()->GetComponentStrage<ModelHandle>();
		for (const auto& [entityId, model] : modelStrage) {
			Render::Model::DrawModel(model.handle);
		}
	}
	// スプライト描画
	if (assetManager_->GetEntityManager()->HasComponentStrage<SpriteData>()) {
		const auto& spriteStrage = assetManager_->GetEntityManager()->GetComponentStrage<SpriteData>();
		for (const auto& [entityId, sprite] : spriteStrage) {
			Render::Sprite::DrawSprite(entityId);
		}
	}
}