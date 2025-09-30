#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include "Force.h"

class PhysicsManager : public Singleton<PhysicsManager> {
	friend class Singleton<PhysicsManager>;
	~PhysicsManager() override = default;

public:
	void Initialize();
	void Update();
	void Finalize();
};