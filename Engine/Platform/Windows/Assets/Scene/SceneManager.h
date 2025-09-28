#pragma once
#include "IScene.h"
#include "Utility/DesignPatterns/Singleton.h"
#include <memory>
#include <string>

class SceneManager final : public Singleton<SceneManager> {
	friend class Singleton<SceneManager>;

public:
	void Initalize();
	void Update();
	void Draw();
	void Finalize();

	void LoadModel(const std::string& modelName);

private:
	std::unique_ptr<IScene> currentScene_;
};