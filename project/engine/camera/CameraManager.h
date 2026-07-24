#pragma once
#include "Camera.h"

#include "memory/SparseSets.h"

#include <cstdint>

namespace QFE::CAMERA {
	/// @brief カメラのハンドルを表す列挙型
	enum class CameraHandle : uint32_t {
		Invalid = UINT32_MAX};

	/// @brief カメラの管理を行うクラス
	class CameraManager final {
	public:
		/// @brief カメラの初期化を行います.ハンドルも初期化されます.
		void Initialize();

		/// @brief カメラの作成を行います.ハンドルが返却されます.
		CameraHandle CreateCamera(float left, float right, float top, float bottom, float nearZ, float farZ, float fov);

		/// @brief あるカメラのあるトランスフォームからビュー行列と投影行列を掛け合わせた行列を取得します
		MATH::Matrix4x4 GetViewProjectionMatrix(CameraHandle handle, const MATH::EulerTransform& cameraTransform, CameraType type);
		/// @brief あるカメラのWVP行列を取得します
		MATH::Matrix4x4 GetWVPMatrix(CameraHandle handle, const MATH::Matrix4x4& worldMatrix, const MATH::EulerTransform& cameraTransform, CameraType type);

	private:
		QFE::SparseSet<Camera> cameras_;
	};

}
