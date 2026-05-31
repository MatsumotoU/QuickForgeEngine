#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include <vector>
#include <unordered_map>
#include "Camera.h"

namespace QFE {

	class CameraManager final : public Singleton<CameraManager> {
		friend class Singleton<CameraManager>;
	public:
		void Initialize();
		void Shutdown();
		void Update();
		void Reset();

		uint32_t AddCamera();
		uint32_t GetMainCameraIndex() const { return mainCameraIndex_; }
		Camera& GetCamera(uint32_t index);
		Camera& GetMainCamera();
		const EulerTransform& GetMainCameraTransform() const;
		std::unordered_map<uint32_t, Camera>& GetAllCameras();
		void SnapToDebugCamera(uint32_t index);

		void SetMainCameraIndex(uint32_t index);
#ifdef QFE_OPTIMIZE_OFF
		void SetActiveDebugCamera(bool isActive) { isActiveDebugCamera_ = isActive; }
		bool IsActiveDebugCamera() const { return isActiveDebugCamera_; }
#endif // QFE_OPTIMIZE_OFF

	private:
		CameraManager() = default;
		~CameraManager() = default;

		bool isActiveDebugCamera_;

		uint32_t mainCameraIndex_;
		std::unordered_map<uint32_t, Camera> cameras_;
		uint32_t nextCameraHandle_;

		EulerTransform dummyCameraTransform_;
	};

}
