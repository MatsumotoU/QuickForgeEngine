#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include <vector>
#include "Camera.h"

class CameraManager final : public Singleton<CameraManager> {
	friend class Singleton<CameraManager>;
public:
	void Initialize();
	void Shutdown();
	void Update();

	uint32_t AddCamera();
	Camera& GetCamera(uint32_t index);
	Camera& GetMainCamera();

private:
	CameraManager() = default;
	~CameraManager() = default;

	uint32_t mainCameraIndex_;
	std::vector<Camera> cameras_;
};