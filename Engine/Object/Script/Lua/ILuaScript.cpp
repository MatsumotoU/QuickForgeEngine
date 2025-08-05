#include "ILuaScript.h"
#include "Math/Transform.h"
#include "Math/Matrix/Matrix4x4.h"
#include "Math/Matrix/Matrix3x3.h"
#include "Math/Matrix/Matrix2x2.h"
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector4.h"
#include "Math/Vector/Vector2.h"
#include "Math/Quaternion/Quaternion.h"

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

ILuaScript::ILuaScript() {
	luaState_.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);

	// Transform
	luaState_.new_usertype<Transform>("Transform",
		sol::constructors<Transform()>(),
		"scale", &Transform::scale,
		"rotate", &Transform::rotate,
		"translate", &Transform::translate
	);
	// Quaternion
	luaState_.new_usertype<Quaternion>("Quaternion",
		sol::constructors<Quaternion()>(),
		"q", &Quaternion::q,
		"ImaginaryPart", &Quaternion::ImaginaryPart,
		"RealPart", &Quaternion::RealPart,
		"Conjugation", &Quaternion::Conjugation,
		"Norm", sol::overload(
			static_cast<float (Quaternion::*)() const>(&Quaternion::Norm),
			static_cast<float (*)(const Quaternion&)>(&Quaternion::Norm)
		),
		"Normalize", sol::overload(
			static_cast<Quaternion(Quaternion::*)() const>(&Quaternion::Normalize),
			static_cast<Quaternion(*)(const Quaternion&)>(&Quaternion::Normalize)
		),
		"Multiply", &Quaternion::Multiply,
		"IndentityQuaternion", &Quaternion::IndentityQuaternion,
		"ConjugationQuaternion", &Quaternion::ConjugationQuaternion,
		"Inverse", &Quaternion::Inverse,
		"MakeRotateAxisAngleQuaternion", &Quaternion::MakeRotateAxisAngleQuaternion,
		"RotateVector", &Quaternion::RotateVector,
		"MakeRotateMatrix", &Quaternion::MakeRotateMatrix,
		"Slerp", &Quaternion::Slerp
	);
	// Vector2
	luaState_.new_usertype<Vector2>("Vector2",
		sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y,
		"Length", &Vector2::Length,
		"Normalize", sol::overload(
			static_cast<Vector2(Vector2::*)() const>(&Vector2::Normalize)
		),
		"Dot", &Vector2::Dot,
		"Cross", &Vector2::Cross,
		"Distance", &Vector2::Distance
	);
	// Vector3
	luaState_.new_usertype<Vector3>("Vector3",
		sol::constructors<Vector3()>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z,
		"Length", &Vector3::Length,
		"Normalize", sol::overload(
			static_cast<Vector3(Vector3::*)() const>(&Vector3::Normalize),
			static_cast<Vector3(*)(const Vector3&)>(&Vector3::Normalize)
		),
		"Dot", &Vector3::Dot,
		"Cross", &Vector3::Cross,
		"Lerp", &Vector3::Lerp,
		"Slerp", &Vector3::Slerp,
		"BezierCurve", &Vector3::BezierCurve,
		"CatmullRom", &Vector3::CatmullRom,
		"Transform", &Vector3::Transform,
		"LookAt", &Vector3::LookAt,
		"Project", &Vector3::Project,
		"Perpendicular", &Vector3::Perpendicular,
		"SphericalToCartesian", &Vector3::SphericalToCartesian,
		"CartesianToSpherical", &Vector3::CartesianToSpherical
	);
	// Vector4
	luaState_.new_usertype<Vector4>("Vector4",
		sol::constructors<Vector4()>(),
		"x", &Vector4::x,
		"y", &Vector4::y,
		"z", &Vector4::z,
		"w", &Vector4::w,
		"Length", &Vector4::Length,
		"Normalize", &Vector4::Normalize,
		"xyz", &Vector4::xyz,
		"Leap", &Vector4::Leap
	);
	// Matrix2x2
	luaState_.new_usertype<Matrix2x2>("Matrix2x2",
		sol::constructors<Matrix2x2()>(),
		"Get", &Matrix2x2::Get,
		"Set", &Matrix2x2::Set,
		"Transpose", sol::overload(
			static_cast<Matrix2x2(Matrix2x2::*)() const>(&Matrix2x2::Transpose),
			static_cast<Matrix2x2(*)(const Matrix2x2&)>(&Matrix2x2::Transpose)
		),
		"Add", &Matrix2x2::Add,
		"Subtract", &Matrix2x2::Subtract,
		"MakeIdentity2x2", &Matrix2x2::MakeIdentity2x2
	);
	// Matrix3x3
	luaState_.new_usertype<Matrix3x3>("Matrix3x3",
		sol::constructors<Matrix3x3()>(),
		"Get", &Matrix3x3::Get,
		"Set", &Matrix3x3::Set,
		"Inverse", sol::overload(
			static_cast<Matrix3x3(Matrix3x3::*)() const>(&Matrix3x3::Inverse),
			static_cast<Matrix3x3(*)(const Matrix3x3&)>(&Matrix3x3::Inverse)
		),
		"Transpose", sol::overload(
			static_cast<Matrix3x3(Matrix3x3::*)() const>(&Matrix3x3::Transpose),
			static_cast<Matrix3x3(*)(const Matrix3x3&)>(&Matrix3x3::Transpose)
		),
		"Add", &Matrix3x3::Add,
		"Subtract", &Matrix3x3::Subtract,
		"Multiply", &Matrix3x3::Multiply,
		"MakeIdentity3x3", &Matrix3x3::MakeIdentity3x3,
		"MakeScaleMatrix", &Matrix3x3::MakeScaleMatrix,
		"MakeRotateMatrix", &Matrix3x3::MakeRotateMatrix,
		"MakeTranslateMatrix", &Matrix3x3::MakeTranslateMatrix,
		"MakeAffineMatrix", &Matrix3x3::MakeAffineMatrix,
		"MakeOrthographicMatrix", &Matrix3x3::MakeOrthographicMatrix,
		"MakeViewportMatrix", &Matrix3x3::MakeViewportMatrix,
		"MakeWvpVpMatrix", &Matrix3x3::MakeWvpVpMatrix
	);
	// Matrix4x4
	luaState_.new_usertype<Matrix4x4>("Matrix4x4",
		sol::constructors<Matrix4x4()>(),
		"Get", &Matrix4x4::Get,
		"Set", &Matrix4x4::Set,
		"Inverse", sol::overload(
			static_cast<Matrix4x4(Matrix4x4::*)() const>(&Matrix4x4::Inverse),
			static_cast<Matrix4x4(*)(const Matrix4x4&)>(&Matrix4x4::Inverse)
		),
		"Transpose", sol::overload(
			static_cast<Matrix4x4(Matrix4x4::*)() const>(&Matrix4x4::Transpose),
			static_cast<Matrix4x4(*)(const Matrix4x4&)>(&Matrix4x4::Transpose)
		),
		"Add", &Matrix4x4::Add,
		"Subtract", &Matrix4x4::Subtract,
		"Multiply", &Matrix4x4::Multiply,
		"MakeIndentity4x4", &Matrix4x4::MakeIndentity4x4,
		"MakeScaleMatrix", &Matrix4x4::MakeScaleMatrix,
		"MakeTranslateMatrix", &Matrix4x4::MakeTranslateMatrix,
		"MakeRotateXMatrix", &Matrix4x4::MakeRotateXMatrix,
		"MakeRotateYMatrix", &Matrix4x4::MakeRotateYMatrix,
		"MakeRotateZMatrix", &Matrix4x4::MakeRotateZMatrix,
		"MakeRotateXYZMatrix", &Matrix4x4::MakeRotateXYZMatrix,
		"MakeAffineMatrix", &Matrix4x4::MakeAffineMatrix,
		"MakeRotateAxisAngle", &Matrix4x4::MakeRotateAxisAngle,
		"DirectionToDirection", &Matrix4x4::DirectionToDirection,
		"MakePerspectiveFovMatrix", &Matrix4x4::MakePerspectiveFovMatrix,
		"MakeOrthographicMatrix", &Matrix4x4::MakeOrthographicMatrix,
		"MakeViewportMatrix", &Matrix4x4::MakeViewportMatrix
	);

#ifdef _DEBUG
	luaState_.set_function("DebugLog", &DebugLogLua);
#endif // _DEBUG
}

sol::state& ILuaScript::GetLuaState() {
	return luaState_;
}
