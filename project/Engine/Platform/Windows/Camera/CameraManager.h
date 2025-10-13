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
	std::vector<Camera>& GetCameras();
	
#ifdef _DEBUG
	void SetActiveDebugCamera(bool isActive) { isActiveDebugCamera_ = isActive; }
	bool IsActiveDebugCamera() const { return isActiveDebugCamera_; }
#endif // _DEBUG

private:
	CameraManager() = default;
	~CameraManager() = default;

#ifdef _DEBUG
	bool isActiveDebugCamera_;
#endif // _DEBUG

	uint32_t mainCameraIndex_;
	std::vector<Camera> cameras_;
};