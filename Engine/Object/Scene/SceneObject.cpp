#include "SceneObject.h"
#include "Base/EngineCore.h"
#include "Model/Model.h"

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

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

}

void SceneObject::Update() {
	// カメラの更新
	mainCamera_.Update();
#ifdef _DEBUG
	debugCamera_.Update();
	mainCamera_ = debugCamera_.camera_;
#endif // _DEBUG

	// シーン更新
	
}

void SceneObject::Draw() {
#ifdef _DEBUG
	engineCore_->GetGraphRenderer()->DrawGrid(50.0f, 50);
#endif // _DEBUG

	// シーン描画
	
}

IScene* SceneObject::GetNextScene() {
	return nullptr;
}
