#pragma once
#include "Base/EngineCore.h"
#include <memory>

class ResultUI {
public:
	ResultUI() = default;
	~ResultUI() = default;
	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

	bool GetSelectedTop() const { return selectedTop_; }
	
	Transform transform_;
	float topOffset_;
	float bottomOffset_;
	bool isCleard_;

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> niceTextModel_;
	std::unique_ptr<Model> retryTextModel_;
	std::unique_ptr<Model> toSelectTextModel_;
	std::unique_ptr<Model> toNextStageTextModel_;
	
	float time_;
	bool selectedTop_;
	float selectInterval_;
};