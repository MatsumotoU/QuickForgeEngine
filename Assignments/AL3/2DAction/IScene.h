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
	virtual std::unique_ptr<IScene> GetNextScene() = 0;

	bool isInitialized_ = false;
protected:
	bool isRequestedExit_;
};