#pragma once

class Model;

class BigMole {
public:
	BigMole() = default;
	~BigMole() = default;

	void DebugImGui();

	void Initialize(Model *model, Model *hangar);
	void Update();
	void Draw();

private:
	Model* model_;
	Model* hangar_;
};

