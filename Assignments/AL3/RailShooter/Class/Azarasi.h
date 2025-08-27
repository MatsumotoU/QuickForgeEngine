#pragma once
#include "Base/EngineCore.h"

class Azarasi {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

	bool reqestMouthOpen_;
	Transform transform_;

private:
	bool isMouthOpen_;
	EngineCore* engineCore_;
	Model normalModel_;
	Model openMouthModel_;
};