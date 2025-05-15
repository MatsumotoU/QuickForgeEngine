#pragma once
class EngineCore;

class GameScene {
public:
	GameScene(EngineCore* engineCore);

public:
	void Initialize();
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;

};