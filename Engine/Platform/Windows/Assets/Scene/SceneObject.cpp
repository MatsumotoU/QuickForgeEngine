#include "SceneObject.h"
#include "Assets/AssetManager.h"
#include <cassert>

#include "Assets/3DModel/Data/ModelHandle.h"

#include "Renderer/ModelRenderer.h"

SceneObject::SceneObject() {
	assetManager_ = nullptr;
	isRequestedExit_ = false;
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
}