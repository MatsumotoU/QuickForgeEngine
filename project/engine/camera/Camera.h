#pragma once
#include "Math/transform/Transform.h"
#include "Math/Matrix/Matrix4x4.h"

namespace QFE::CAMERA {
	/// @brief カメラの種類を表す列挙型
	enum class CameraType {
		Perspective,// 透視投影(3D)
		Orthographic// 平行投影(2D)
	};
	/// @brief あるトランスフォームからカメラ自体を表すクラス
	class Camera final {
	public:
		/// @brief カメラの初期化を行います
		void Initialize(float left, float right, float top, float bottom, float nearZ, float farZ,float fov);
		/// @brief 透視投影行列を作成します
		void CreatePerspectiveMatrix(float fovY, float aspectRatio, float nearZ, float farZ);
		/// @brief 平行投影行列を作成します
		void CreateOrthographicMatrix(float left, float right, float top, float bottom, float nearZ, float farZ);

		/// @brief ビュー行列と投影行列を掛け合わせた行列を取得します
		MATH::Matrix4x4 GetViewProjectionMatrix(const MATH::Transform& cameraTranform, CameraType type) const;

	private:
		float fovY_;
		float aspectRatio_;
		float nearZ_;
		float farZ_;
		// 画面の左、右、上、下の座標
		float left_;
		float right_;
		float top_;
		float bottom_;
	};

}
