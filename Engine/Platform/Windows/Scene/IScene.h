#pragma once
#include <string>

class IScene {
public:
	virtual ~IScene() = default;
	// Initialize the scene
	virtual void Initialize() = 0;
	// Update the scene
	virtual void Update() = 0;
	// Draw the scene
	virtual void Draw() = 0;

	std::string& GetSceneName() { return sceneName_; }
	void SetSceneName(const std::string& name) { sceneName_ = name; }
	bool GetReqesytedExit() { return isRequestedExit_; }

protected:
	std::string sceneName_;
	bool isRequestedExit_;
};