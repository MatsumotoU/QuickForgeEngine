#pragma once
class IScene {
public:
	virtual ~IScene() = default;
	// Initialize the scene
	virtual void Initialize() = 0;
	// Update the scene
	virtual void Update() = 0;
	// Draw the scene
	virtual void Draw() = 0;

	bool GetReqesytedExit() { return isRequestedExit_; }
	virtual IScene* GetNextScene() = 0;
	int GetSceneID() const { return sceneID_; }

protected:
	int sceneID_ = -1;
	bool isRequestedExit_;
	nlohmann::json* sceneData_;
};