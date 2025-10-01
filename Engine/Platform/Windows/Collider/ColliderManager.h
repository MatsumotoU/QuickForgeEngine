#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include "Data/SphereColliderData.h"

class ColliderManager final : public Singleton<ColliderManager> {
	friend class Singleton<ColliderManager>;
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	bool isRunning = false;

private:
	bool isCollision(const Sphere& sphere1, const Sphere& sphere2);
};