#include "SceneObject.h"
#include "Base/EngineCore.h"

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#include "Object/System/Entity/AppSys/SceneEntityViewer.h"
#endif // _DEBUG

#include "Object/System/Entity/AppSys/EntityLoader.h"
#include "Object/System/Model/AppSys/LoadModelForEntities.h"
#include "Object/System/Model/AppSys/DrawMesh.h"
#include "Object/System/Entity/AppSys/TransformUpdate.h"
#include "Object/System/Entity/AppSys/PearentEntityMove.h"

SceneObject::SceneObject(EngineCore* enginecore, const std::string& sceneName) 
#ifdef _DEBUG
	: debugCamera_(enginecore)
#endif 
{
	sceneName_ = sceneName;
	engineCore_ = enginecore;
	mainCamera_.Initialize(enginecore->GetWinApp());
#ifdef _DEBUG
	debugCamera_.Initialize(enginecore);
#endif // _DEBUG

	enginecore->GetGraphRenderer()->SetCamera(&mainCamera_);
}

SceneObject::~SceneObject() {
	engineCore_->GetGraphRenderer()->DeleteCamera(&mainCamera_);
}

void SceneObject::Initialize() {
	//EntityLoader::LoadEntities(engineCore_, entityManager_, "SampleScene.json");

	//entityManager_.CreateEntity(); // 空のエンティティを作成
	//LoadModelForEntities::Load(engineCore_, entityManager_, 2, "axis.obj");
}

void SceneObject::Update() {
	// カメラの更新
	mainCamera_.Update();
#ifdef _DEBUG
	debugCamera_.Update();
	mainCamera_ = debugCamera_.camera_;
#endif // _DEBUG

	// エンティティ更新
	ParentEntityMove::Update(entityManager_);
	TransformUpdate::Update(entityManager_,mainCamera_);
}

void SceneObject::Draw() {
#ifdef _DEBUG
	SceneEntityViewer::DisplayEntities(entityManager_);
	engineCore_->GetGraphRenderer()->DrawGrid(50.0f, 50);
#endif // _DEBUG

	// シーン描画
	DrawMesh::Draw(engineCore_, entityManager_, mainCamera_);
}

IScene* SceneObject::GetNextScene() {
	return nullptr;
}
