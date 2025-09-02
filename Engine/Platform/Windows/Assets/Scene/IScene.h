#pragma once
class IScene{
public:
	IScene();
	virtual ~IScene() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	bool isReqestSceneChange();
	virtual IScene* GetNextScene() = 0;

protected:
	bool reqestSceneChange_;
};