#include "IScene.h"

IScene::IScene() : reqestSceneChange_(false) {}

bool IScene::isReqestSceneChange() {
	return reqestSceneChange_;
}

